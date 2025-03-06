#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <tinycthread.h> // C11 threads but cross-platform (instead of using thread, pthread, etc.)
#include <raylib.h>
#include <rlgl.h>

#define CLAY_IMPLEMENTATION
#include <clay.h>

#include "helpers/include.h" // the standard include file with a lot of basic functions
#include "rendering/graphical/ui/ui_renderer.h"
#include "rendering/graphical/ui/base_ui.h"

#define SAMPLE_RATE 44100
#define BUFFER_SIZE 512
#define AUDIO_THREAD_PRIORITY 0 // 0 is considered default priority, if clicking occurs try setting to 1 which raises it to critical

typedef struct {
    float phase1;
    float phase2;
    mtx_t mutex;
    bool running;
    float* buffer;
} AudioState;

AudioState audio_state = {0};
AudioStream stream = {0};
thrd_t audio_thread;

float phase1 = 0.0f;
float phase2 = 0.0f;

float frequency1 = 440.0f;
float frequency2 = 660.0f;

void GenerateSineWave(float *buffer, int frames) {
    float phaseStep1 = 2 * PI * frequency1 / SAMPLE_RATE;
    float phaseStep2 = 2 * PI * frequency2 / SAMPLE_RATE;

    for (int i = 0; i < frames; i++) {
        float sample1 = sinf(phase1);
        // Sample2 is a triangle wave
        float sample2 = 2.0f * fabsf(2.0f * (phase2 / (2 * PI) - floorf(phase2 / (2 * PI) + 0.5f))) - 1.0f;

        buffer[i] = (sample1 + sample2) * 0.5f;  // Mix the two waves and reduce volume

        phase1 += phaseStep1;
        phase2 += phaseStep2;

        if (phase1 > 2 * PI) phase1 -= 2 * PI;
        if (phase2 > 2 * PI) phase2 -= 2 * PI;
    }
}

void DrawWaveform(const float *buffer, int bufferSize, int screenWidth, int screenHeight) {
    int centerY = screenHeight / 2;
    Vector2 points[bufferSize];

    for (int i = 0; i < bufferSize; i++) {
        int x = (i * screenWidth) / bufferSize;
        int y = centerY + (int)(buffer[i] * ((float)screenHeight / 2)); // Scale the sample

        points[i] = (Vector2){x, y};
    }

    // We need to draw a line between each point
    rlBegin(RL_LINES);

    rlColor4ub(0, 255, 0, 255);

    for (int i = 0; i < bufferSize - 1; i++) {
        rlVertex2f(points[i].x, points[i].y);
        rlVertex2f(points[i + 1].x, points[i + 1].y);
    }

    rlEnd();
}

int AudioThread(void* arg) {
    AudioState* state = (AudioState*)arg;
    float buffer[BUFFER_SIZE];
    audio_state.buffer = buffer;

    while (state->running) {
        mtx_lock(&state->mutex);

        if (IsAudioStreamProcessed(stream)) {
            GenerateSineWave(buffer, BUFFER_SIZE);
            UpdateAudioStream(stream, buffer, BUFFER_SIZE);
        }

        mtx_unlock(&state->mutex);
    }
    return 0;
}

int main(void) {
    Init(1024 * 1024 * 4); // Initialize the Helper Library with a 4MB arena
    InitWindow(1920, 1080, "Hello, World!");
    SetTargetFPS(250);

    InitUI(1920, 1080);
    SetUIRenderFunction(RenderMainUI);

    SetWindowState(FLAG_WINDOW_RESIZABLE);

    InitAudioDevice();
    SetAudioStreamBufferSizeDefault(BUFFER_SIZE);
    stream = LoadAudioStream(SAMPLE_RATE, 32, 1);
    PlayAudioStream(stream);

    mtx_init(&audio_state.mutex, mtx_plain);
    audio_state.running = false; // TODO: enable this again lol

    thrd_create(&audio_thread, AudioThread, &audio_state);

    while (!WindowShouldClose()) {
        Clay_RenderCommandArray commands = DrawUI(GetScreenWidth(), GetScreenHeight(), GetMouseX(), GetMouseY(), IsMouseButtonDown(MOUSE_LEFT_BUTTON), GetMouseWheelMoveV().x, GetMouseWheelMoveV().y, GetFrameTime());

        BeginDrawing();
        ClearBackground(BLACK);

        if (IsKeyPressed(KEY_F8)){
            Clay_SetDebugModeEnabled(true);
        }

        RaylibRenderUI(commands);

        EndDrawing();
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