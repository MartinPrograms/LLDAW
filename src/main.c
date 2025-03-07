#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <raylib.h>
#include <tinycthread.h> // C11 threads but cross-platform (instead of using thread, pthread, etc.)
#include <rlgl.h>

#define CLAY_IMPLEMENTATION
#include <clay.h>
#include <string.h>

#include "helpers/include.h" // the standard include file with a lot of basic functions
#include "rendering/graphical/ui/ui_renderer.h"
#include "rendering/graphical/ui/base_ui.h"

void CalculateBuffer(float *buffer, int bufferSize) {
    for (int i = 0; i < bufferSize; i++) {
        float sumL = 0;
        float sumR = 0;

        for (int j = 0; j < audio_state.generatorState.generatorCount; j++) {
            Generator *generator = &audio_state.generatorState.generators[j];
            sumL += generator->generate(generator, true, false);
            sumR += generator->generate(generator, false, true); // we advance the phase here!
        }
        sumL = fmin(fmax(sumL, -1.0f), 1.0f);
        sumR = fmin(fmax(sumR, -1.0f), 1.0f);
        buffer[2*i] = sumR; // inverted because raylib uses a different convention
        buffer[2*i + 1] = sumL;

        audio_state.bigFifoBuffer[audio_state.bigFifoBufferIndex] = buffer[i];
        audio_state.bigFifoBufferIndex = (audio_state.bigFifoBufferIndex + 1) % BIG_FIFO_BUFFER_SIZE;

        audio_state.smallFifoBuffer[audio_state.smallFifoBufferIndex] = buffer[i];
        audio_state.smallFifoBufferIndex = (audio_state.smallFifoBufferIndex + 1) % SMALL_FIFO_BUFFER_SIZE;
    }
}

int AudioThread(void* arg) {
    AudioState* state = (AudioState*)arg;

    // Thanks microsoft, i could not have had a worse time trying to get this to work :(

    state->buffer = (float*)malloc(BUFFER_SIZE * 2 * sizeof(float)); // times 2 because we have 2 channels
    state->bigFifoBuffer = (float*)malloc(BIG_FIFO_BUFFER_SIZE * sizeof(float));
    state->smallFifoBuffer = (float*)malloc(SMALL_FIFO_BUFFER_SIZE * sizeof(float));

    if (!state->buffer || !state->bigFifoBuffer || !state->smallFifoBuffer) {
        fprintf(stderr, "Memory allocation failed\n");
        return -1;
    }

    while (state->running) {
        if (state->reset) {
            memset(state->buffer, 0, sizeof(float) * BUFFER_SIZE * 2); // times 2 because we have 2 channels
            state->reset = false;

            memset(state->bigFifoBuffer, 0, sizeof(float) * BIG_FIFO_BUFFER_SIZE);
            state->bigFifoBufferIndex = 0;

            memset(state->smallFifoBuffer, 0, sizeof(float) * SMALL_FIFO_BUFFER_SIZE);
            state->smallFifoBufferIndex = 0;

            continue;
        }

        if (state->paused) {
            continue;
        }

        mtx_lock(&state->mutex);

        if (IsAudioStreamProcessed(stream)) {
            CalculateBuffer(state->buffer, BUFFER_SIZE);
            UpdateAudioStream(stream, state->buffer, BUFFER_SIZE);
        }

        mtx_unlock(&state->mutex);
    }
    return 0;
}

void play(__unused void* userdata) {
    // mark userdata as optional, so the warning does not show up
    mtx_lock(&audio_state.mutex);
    audio_state.paused = false;
    mtx_unlock(&audio_state.mutex);
}

void pauseCallback(__unused void* userdata) {
    mtx_lock(&audio_state.mutex);
    audio_state.paused = true;
    mtx_unlock(&audio_state.mutex);
}

void stop(__unused void* userdata) {
    mtx_lock(&audio_state.mutex);
    audio_state.paused = true;
    audio_state.reset = true;
    mtx_unlock(&audio_state.mutex);
}

int main(void) {
    Init(1024 * 1024 * 4); // Initialize the Helper Library with a 4MB arena
    InitWindow(1920, 1080, "LLDAW");
    SetTargetFPS(240);

    SetWindowState(FLAG_WINDOW_RESIZABLE);

    InitAudioDevice();
    SetAudioStreamBufferSizeDefault(BUFFER_SIZE);
    stream = LoadAudioStream(SAMPLE_RATE, 32, 2);
    PlayAudioStream(stream);

    audio_state = (AudioState) {
        .running = true,
        .paused = true,
        .reset = false,
        .generatorState = generator_init(64) // cpu issues will probably happen around #500 generators
    };

    // Add a generator at 80 hz, sine wave left pan
    generator_add(&audio_state.generatorState, (Generator) {
        .frequency = 80,
        .phase = 0,
        .waveform = SINE,
        .amplitude = 0.5,
        .generate = GenerateWaveform,
        .panning = -1
    });

    // Add a generator at 80 * 4 hz, square wave right pan
    generator_add(&audio_state.generatorState, (Generator) {
        .frequency = 80 * 4,
        .phase = 0,
        .waveform = SINE,
        .amplitude = 0.5,
        .generate = GenerateWaveform,
        .panning = 1
    });


    mtx_init(&audio_state.mutex, mtx_plain);
    audio_state.running = true;
    audio_state.paused = true;

    PlayCallback = play;
    PauseCallback = pauseCallback;
    StopCallback = stop;

    thrd_create(&audio_thread, AudioThread, &audio_state);

    InitUI(GetScreenWidth(), GetScreenHeight());
    SetUIRenderFunction(RenderMainUI);

    while (!WindowShouldClose()) {

         Clay_RenderCommandArray commands = DrawUI(GetScreenWidth(),
                                                  GetScreenHeight(),
                                                  GetMouseX(),
                                                  GetMouseY(),
                                                  IsMouseButtonDown(MOUSE_LEFT_BUTTON),
                                                  GetMouseWheelMoveV().x,
                                                  GetMouseWheelMoveV().y,
                                                  GetFrameTime());

        BeginDrawing();
        ClearBackground(BLACK);

        if (IsKeyPressed(KEY_F8)){
            Clay_SetDebugModeEnabled(true);
        }

        if (IsKeyPressed(KEY_SPACE)) {
            if (audio_state.paused) {
                play(nullptr);
            } else {
                stop(nullptr);
            }
        }

        RaylibRenderUI(commands);

        EndDrawing();

        arena_reset(frame_arena); // Reset the frame arena
    }

    printf("Closing...\n");

    audio_state.running = false;
    thrd_join(audio_thread, NULL);
    mtx_destroy(&audio_state.mutex);

    UnloadAudioStream(stream);
    CloseAudioDevice();

    CloseWindow();

    Destroy(); // Important to reset the console mode on macOS/linux and free the arena

    return 0;
}