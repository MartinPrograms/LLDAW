#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "../libs/tinycthread/tinycthread.h" // C11 threads but cross-platform (instead of using thread, pthread, etc.)

#include "helpers/include.h" // the standard include file with a lot of basic functions

#include "../libs/raylib/raylib.h"

#define SAMPLE_RATE 44100
#define BUFFER_SIZE 1024
#define AUDIO_THREAD_PRIORITY 0 // 0 is considered default priority, if clicking occurs try setting to 1 which raises it to critical

typedef struct {
    float phase1;
    float phase2;
    mtx_t mutex;
    bool running;
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
        float sample2 = sinf(phase2);

        buffer[i] = (sample1 + sample2) * 0.5f;  // Mix the two waves and reduce volume

        phase1 += phaseStep1;
        phase2 += phaseStep2;

        if (phase1 > 2 * PI) phase1 -= 2 * PI;
        if (phase2 > 2 * PI) phase2 -= 2 * PI;
    }
}

int AudioThread(void* arg) {
    AudioState* state = (AudioState*)arg;
    float buffer[BUFFER_SIZE];

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
    SetTargetFPS(60);
    InitAudioDevice();
    SetAudioStreamBufferSizeDefault(BUFFER_SIZE);

    stream = LoadAudioStream(SAMPLE_RATE, 32, 1);
    PlayAudioStream(stream);

    mtx_init(&audio_state.mutex, mtx_plain);
    audio_state.running = true;

    thrd_create(&audio_thread, AudioThread, &audio_state);

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(RAYWHITE);
        DrawText("Frequency 1: ", 10, 10, 20, BLACK);
        DrawText(TextFormat("%f", frequency1), 150, 10, 20, BLACK);
        DrawText("Frequency 2: ", 10, 40, 20, BLACK);
        DrawText(TextFormat("%f", frequency2), 150, 40, 20, BLACK);

        if (IsKeyDown(KEY_UP)) {
            frequency1 = frequency1 + 100.0f * GetFrameTime();
            frequency2 = frequency2 + 100.0f * GetFrameTime();
        }

        if (IsKeyDown(KEY_DOWN)) {
            frequency1 = frequency1 - 100.0f * GetFrameTime();
            frequency2 = frequency2 - 100.0f * GetFrameTime();
        }

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