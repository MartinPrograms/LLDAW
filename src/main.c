#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <tinycthread.h> // C11 threads but cross-platform (instead of using thread, pthread, etc.)
#include <raylib.h>
#include <rlgl.h>

#define CLAY_IMPLEMENTATION
#include <clay.h>
#include <string.h>

#include "helpers/include.h" // the standard include file with a lot of basic functions
#include "rendering/graphical/ui/ui_renderer.h"
#include "rendering/graphical/ui/base_ui.h"

void CalculateBuffer(float *buffer, int frames) {
    for (int i = 0; i < frames; i++) {
        for (int j = 0; j < audio_state.generatorState.generatorCount; j++) {
            Generator generator = audio_state.generatorState.generators[j];
            buffer[i] += generator.generate(generator.frequency, generator.phase, generator.amplitude, generator.waveform);
        }

        audio_state.bigFifoBuffer[audio_state.bigFifoBufferIndex] = buffer[i];
        audio_state.bigFifoBufferIndex = (audio_state.bigFifoBufferIndex + 1) % BIG_FIFO_BUFFER_SIZE;

        audio_state.smallFifoBuffer[audio_state.smallFifoBufferIndex] = buffer[i];
        audio_state.smallFifoBufferIndex = (audio_state.smallFifoBufferIndex + 1) % SMALL_FIFO_BUFFER_SIZE;
    }
}

int AudioThread(void* arg) {
    AudioState* state = (AudioState*)arg;
    float buffer[BUFFER_SIZE];
    audio_state.buffer = buffer;
    float fifoBuffer[BIG_FIFO_BUFFER_SIZE];
    audio_state.bigFifoBuffer = fifoBuffer;
    float smallFifoBuffer[SMALL_FIFO_BUFFER_SIZE];
    audio_state.smallFifoBuffer = smallFifoBuffer;

    while (state->running) {
        if (state->reset) {
            memset(buffer, 0, sizeof(buffer));
            state->reset = false;

            memset(fifoBuffer, 0, sizeof(fifoBuffer));
            state->bigFifoBufferIndex = 0;

            memset(smallFifoBuffer, 0, sizeof(smallFifoBuffer));
            state->smallFifoBufferIndex = 0;

            continue;
        }

        if (state->paused) {
            continue;
        }

        mtx_lock(&state->mutex);

        if (IsAudioStreamProcessed(stream)) {
            CalculateBuffer(buffer, BUFFER_SIZE);
            UpdateAudioStream(stream, buffer, BUFFER_SIZE);
        }

        mtx_unlock(&state->mutex);
    }
    return 0;
}

void play() {
    mtx_lock(&audio_state.mutex);
    audio_state.paused = false;
    mtx_unlock(&audio_state.mutex);
}

void pauseCallback() {
    mtx_lock(&audio_state.mutex);
    audio_state.paused = true;
    mtx_unlock(&audio_state.mutex);
}

void stop() {
    mtx_lock(&audio_state.mutex);
    audio_state.paused = true;
    audio_state.reset = true;
    mtx_unlock(&audio_state.mutex);
}

int main(void) {
    Init(1024 * 1024 * 4); // Initialize the Helper Library with a 4MB arena
    InitWindow(1920, 1080, "LLDAW");
    SetTargetFPS(240);

    SetWindowState(FLAG_WINDOW_RESIZABLE | FLAG_WINDOW_MAXIMIZED);

    InitAudioDevice();
    SetAudioStreamBufferSizeDefault(BUFFER_SIZE);
    stream = LoadAudioStream(SAMPLE_RATE, 32, 1);
    PlayAudioStream(stream);

    audio_state = (AudioState) {
        .running = true,
        .paused = true,
        .reset = false,
        .generatorState = generator_init(10) // 10 generators max
    };

    generator_add(&audio_state.generatorState,
                  (Generator) {.waveform = SINE, .frequency = 440, .amplitude = 0.5f, .generate = GenerateWaveform});

    mtx_init(&audio_state.mutex, mtx_plain);
    audio_state.running = true;
    audio_state.paused = true;

    PlayCallback = play;
    PauseCallback = pauseCallback;
    StopCallback = stop;

    thrd_create(&audio_thread, AudioThread, &audio_state);

    InitUI(1920, 1080);
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
                play();
            } else {
                stop();
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