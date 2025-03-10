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
    generator_add(&audio_state.generator_state, (Generator) {
        .frequency = 220,
        .phase = 0,
        .waveform = SAWTOOTH,
        .amplitude = 1,
        .generate = GenerateWaveform,
        .panning = -0.f,
        .unison = 4,
        .unison_detune = 0.2f,
        .phase_randomization = 1.0f, // 100% randomization
        .envelope = adsr_envelope_basic()
    });

    generator_add(&audio_state.generator_state, (Generator) {
        .frequency = 220,
        .phase = 0,
        .waveform = TRIANGLE,
        .amplitude = 1,
        .generate = GenerateWaveform,
        .panning = -0.f,
        .unison = 1,
        .unison_detune = 0.2f,
        .phase_randomization = 1.0f, // 100% randomization
        .envelope = (AdsrEnvelope){
            .attack = {0.01f, 0.5f},
            .decay = {0.2f, 0.8f},
            .sustain = 0.0f,
            .release = {0.01f, 0.5f},
        }
    });

    adsr_cache_envelope(&audio_state.generator_state.generators[1].envelope, default_arena, SAMPLE_RATE);

    // basic C major scale
    int rootC = 60;
    float bpm = 80;

    sequencer_add_note((Note) {
            .generator_index = 0,
            .frequency = midi_note_to_frequency(rootC - 24 + 2),
            .amplitude = 0.5f,
            .pan = 0.0f,
            .start_sample = 0,
            .end_sample = samples_from_bpm_time_component(bpm, SAMPLE_RATE, WHOLE) * 8,
            .active = true,
            .midi_note = rootC - 24 + 2
    });

    sequencer_add_note((Note) {
            .generator_index = 0,
            .frequency = midi_note_to_frequency(rootC - 24 + 5),
            .amplitude = 0.5f,
            .pan = 0.0f,
            .start_sample = samples_from_bpm_time_component(bpm, SAMPLE_RATE, WHOLE) * 8,
            .end_sample = samples_from_bpm_time_component(bpm, SAMPLE_RATE, WHOLE) * 16,
            .active = true,
            .midi_note = rootC - 24 + 5
    });

    int dnotesminor[] = {NOTE_D, NOTE_A, NOTE_F, NOTE_G};
    for (int i = 0; i < 64; i++) {
        // arpeggio in D scale
        sequencer_add_note((Note) {
                .generator_index = 1,
                .frequency = midi_note_to_frequency(note_to_midi(dnotesminor[i % 4], 5)),
                .amplitude = 0.5f,
                .pan = 0.0f,
                .start_sample = samples_from_bpm_time_component(bpm, SAMPLE_RATE, QUARTER) * i,
                .end_sample = samples_from_bpm_time_component(bpm, SAMPLE_RATE, QUARTER) * (i + 1),
                .active = true,
                .midi_note = note_to_midi(dnotesminor[i % 4], 5)
        });
    }

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

    return 0;
}