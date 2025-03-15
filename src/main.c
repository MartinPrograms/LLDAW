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
#include "helpers/audio/audio_math.h"
#include "rendering/graphical/ui/ui_renderer.h"
#include "rendering/graphical/ui/base_ui.h"
#include "rendering/audio/audio_processor.h"

#include "sequencing/sequencer.h"

int main(void) {
    init_math(); // This does some stuff for precomputed tables and such

    Init(1024 * 1024 * 4); // Initialize the Helper Library with a 4MB arena
    InitWindow(1920, 1080, "LLDAW");
    SetTargetFPS(240);

    SetWindowState(FLAG_WINDOW_RESIZABLE | FLAG_WINDOW_MAXIMIZED);

    InitAudio();
    ////////////////////////////////////////////////////////////////////////////////////////
    // Create the song:
    sequencer_init();
    sequencer.settings.bpm = 80;
    generator_add(&audio_state.generator_state, (Generator) {
        .frequency = 220,
        .phase = 0,
        .waveform = SINE,
        .amplitude = 1,
        .generate = GenerateWaveform,
        .panning = -0.5f,
        .unison = 8,
        .unison_detune = 0.2f,
        .phase_randomization = 1.0f, // 100% randomization
        .envelope = adsr_envelope_basic()
    });

    generator_add(&audio_state.generator_state, (Generator) {
        .frequency = 220,
        .phase = 0,
        .waveform = SINE,
        .amplitude = 1,
        .generate = GenerateWaveform,
        .panning = 0.5f,
        .unison = 3,
        .unison_detune = 0.1f,
        .phase_randomization = 1.0f, // 100% randomization
        .envelope = adsr_envelope_basic()
    });

    adsr_cache_envelope(&audio_state.generator_state.generators[1].envelope, default_arena, SAMPLE_RATE);

    ////////////////////////////////////////////////////////////////////////////////////////

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
                pauseCallback(nullptr);
            }
        }

        RaylibRenderUI(commands);

        EndDrawing();

        arena_reset(frame_arena); // Reset the frame arena
    }

    printf("Closing...\n");

    audio_state.running = false;

    cnd_broadcast(&audio_state.resume_processing_cnd); // otherwise the audio thread will never wake up, and the program will hang
    cnd_broadcast(&audio_state.resume_playback_cnd);

    thrd_join(audio_playback_thread, NULL);
    thrd_join(audio_process_thread, NULL);

    mtx_destroy(&audio_state.state_mutex);
    mtx_destroy(&audio_state.processing_mutex);

    UnloadAudioStream(stream);
    CloseAudioDevice();

    CloseWindow();

    Destroy(); // Important to reset the console mode on macOS/linux and free the arena

    exit(0); // For some reason this mf program does not close properly without this, yes my code is perfect

    return 0;
}