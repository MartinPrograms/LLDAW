#include "components.h"
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdio.h>
#include <raylib.h>
#include "base_ui.h"

#include "../../../helpers/audio/audio_math.h"

void testing(){
    printf("Test\n");
}

void LeftPanel(){
    CLAY({
             .id = CLAY_ID("LeftPanel"),
             .layout = {
                .sizing = {CLAY_SIZING_PERCENT(0.16), CLAY_SIZING_GROW()},
                .padding = CLAY_PADDING_ALL(STANDARD_PADDING),
                .childGap = STANDARD_GAP,
                .layoutDirection = CLAY_TOP_TO_BOTTOM
        },
             .backgroundColor = COLOR_SCHEME_BACKGROUND_SECONDARY,
             .cornerRadius = CLAY_CORNER_RADIUS(STANDARD_CORNER_RADIUS)
         }) {
        CLAY({
                 .id = CLAY_ID("LeftPanelTitle"), .layout = {.sizing = {CLAY_SIZING_GROW(),
                                                                       CLAY_SIZING_FIT()}, .padding = CLAY_PADDING_ALL(STANDARD_PADDING),.childAlignment = {CLAY_ALIGN_X_CENTER, CLAY_ALIGN_Y_TOP}},
             }) {
            CLAY_TEXT(GetString("Settings"), CLAY_TEXT_CONFIG(
                    {.textColor = COLOR_SCHEME_TEXT, .fontSize = 16, .letterSpacing = 0, .wrapMode = CLAY_TEXT_WRAP_NONE}));
        }

        draw_sequencer_settings();
    }
}

void MiddlePanel() {
    CLAY({
             .id = CLAY_ID("MiddlePanel"),
             .layout = {
                .sizing = {CLAY_SIZING_GROW(), CLAY_SIZING_GROW()},
                .padding = CLAY_PADDING_ALL(STANDARD_PADDING),
                .childGap = STANDARD_GAP,
                .layoutDirection = CLAY_TOP_TO_BOTTOM
        },
             .backgroundColor = COLOR_SCHEME_BACKGROUND_TERTIARY,
             .cornerRadius = CLAY_CORNER_RADIUS(STANDARD_CORNER_RADIUS)
         }) {

        draw_sequencer(&sequencer);
    }
}

void RightPanel(){
    CLAY({
             .id = CLAY_ID("RightPanel"),
             .layout = {
                .sizing = {CLAY_SIZING_PERCENT(0.15), CLAY_SIZING_GROW()},
                .padding = CLAY_PADDING_ALL(STANDARD_PADDING),
                .childGap = STANDARD_GAP,
                .layoutDirection = CLAY_TOP_TO_BOTTOM
        },
             .backgroundColor = COLOR_SCHEME_BACKGROUND_SECONDARY,
             .cornerRadius = CLAY_CORNER_RADIUS(STANDARD_CORNER_RADIUS)

         }) {
        CLAY({
                 .id = CLAY_ID("RightPanelText"), .layout = {.sizing = {CLAY_SIZING_GROW(),
                                                                        CLAY_SIZING_FIT()}, .padding = CLAY_PADDING_ALL(STANDARD_PADDING)},
             }) {
            CLAY_TEXT(GetString("Right Panel"), CLAY_TEXT_CONFIG(
                    {.textColor = COLOR_SCHEME_TEXT, .fontSize = 16, .letterSpacing = 0, .wrapMode = CLAY_TEXT_WRAP_NONE}));
        }
    }
}

void BaseContainer(){
    CLAY({
             .id = CLAY_ID("BaseContainer"),
             .layout = {
                .sizing = {CLAY_SIZING_GROW(), CLAY_SIZING_GROW()},
                .padding = CLAY_PADDING_ALL(0),
                .childGap = STANDARD_GAP,
                .layoutDirection = CLAY_LEFT_TO_RIGHT
        },
             .backgroundColor = COLOR_SCHEME_BACKGROUND,
             .cornerRadius = CLAY_CORNER_RADIUS(STANDARD_CORNER_RADIUS)

         }) {
        LeftPanel();
        MiddlePanel();
        RightPanel();
    }
}

STRING GetTopbarStats() {
    // Begin memory stats:
    STRING stats = StringCreate("LTM: ", frame_arena);
    char *ltmString = arena_alloc(frame_arena, 10);
    snprintf(ltmString, 10, "%.2f", percentage_arena_used(default_arena) * 100);
    stats = StringConcat(&stats, ltmString);
    stats = StringConcat(&stats, "%");

    // Now instead of adding a new line, i wanna save some space, so for STM (short term memory) stats and ABM (audio buffer memory) stats, I'll just add a space
    stats = StringConcat(&stats, " ");
    stats = StringConcat(&stats, "STM: ");
    char *stmString = arena_alloc(frame_arena, 10);
    snprintf(stmString, 10, "%.2f", percentage_arena_used(frame_arena) * 100);
    stats = StringConcat(&stats, stmString);
    stats = StringConcat(&stats, "%");

    stats = StringConcat(&stats, " ");
    stats = StringConcat(&stats, "ABM: ");
    char *abmString = arena_alloc(frame_arena, 10);
    snprintf(abmString, 10, "%.2f", percentage_arena_used(buffer_arena) * 100);
    stats = StringConcat(&stats, abmString);
    stats = StringConcat(&stats, "%");

    // Begin buffer index stat (audio state)
    stats = StringConcat(&stats, "\n");
    stats = StringConcat(&stats, "Buffer index: ");
    char *bufferIndexString = arena_alloc(frame_arena, 10);
    snprintf(bufferIndexString, 10, "%d", audio_state.audio_buffers.buffer_count);
    stats = StringConcat(&stats, bufferIndexString);

    // Append the buffer render time as (time: x)
    stats = StringConcat(&stats, " (");
    char *timeString = arena_alloc(frame_arena, 10);
    snprintf(timeString, 10, "%.2f", audio_state.time_to_render_buffer * 1000);
    stats = StringConcat(&stats, timeString);
    stats = StringConcat(&stats, "ms)");

    if (audio_state.audio_buffers.buffer_count >= AUDIO_BUFFER_COUNT) {
        // This means it's full and waiting, append (waiting)
        stats = StringConcat(&stats, " (waiting)");
    }else
    if (audio_state.audio_buffers.buffer_count <= 0) {
        // This is an underrun, append (underrun)
        stats = StringConcat(&stats, " (underrun)");
    }else {
        // Its currently rendering, append (rendering)
        stats = StringConcat(&stats, " (rendering)");
    }

    if (audio_state.paused) {
        // This means it's paused, append (paused)
        stats = StringConcat(&stats, " (paused)");
    }

    // End buffer index stat

    return stats;
}

void TopBar() {
    CLAY({
             .id = CLAY_ID("TopBar"),
             .layout = {
                .sizing = {CLAY_SIZING_GROW(), CLAY_SIZING_FIT()},
                .padding = CLAY_PADDING_ALL(STANDARD_PADDING),
                .childGap = STANDARD_GAP,
                .childAlignment = {CLAY_ALIGN_X_LEFT, CLAY_ALIGN_Y_CENTER},
                .layoutDirection = CLAY_LEFT_TO_RIGHT
        },
             .backgroundColor = COLOR_SCHEME_BACKGROUND_SECONDARY,
             .cornerRadius = CLAY_CORNER_RADIUS(STANDARD_CORNER_RADIUS)
         }) {

        STRING title = StringCreate("LLDAW", frame_arena);
        title = StringConcat(&title, " - ");
        title = StringConcat(&title, VERSION);

        CLAY({
                 .id = CLAY_ID("TopBarText"), .layout = {.sizing = {CLAY_SIZING_FIT(),
                                                                    CLAY_SIZING_GROW()}, .padding = CLAY_PADDING_ALL(STANDARD_PADDING),
                     .childAlignment = {CLAY_ALIGN_X_LEFT, CLAY_ALIGN_Y_CENTER}},
            .backgroundColor = COLOR_SCHEME_BACKGROUND_DARK,
            .cornerRadius = CLAY_CORNER_RADIUS(STANDARD_CORNER_RADIUS)
             }) {
            CLAY_TEXT(GetString(title.data), CLAY_TEXT_CONFIG(
                    {.textColor = COLOR_SCHEME_TEXT, .fontSize = 32, .fontId = 1, .letterSpacing = 0, .wrapMode = CLAY_TEXT_WRAP_NONE}));
        }

        CLAY({
                 .id = CLAY_ID("PlaybackControls"), // think play, pause, stop
                 .layout = {
                    .sizing = {CLAY_SIZING_GROW(), CLAY_SIZING_GROW()},
                    .padding = CLAY_PADDING_ALL(0),
                    .childGap = STANDARD_GAP,
                    .layoutDirection = CLAY_LEFT_TO_RIGHT,
                    .childAlignment = {CLAY_ALIGN_X_LEFT, CLAY_ALIGN_Y_CENTER}
            }
             }) {
            CreateButton("Play", NULL, PlayCallback);
            CreateButton("Pause", NULL, PauseCallback);
            CreateButton("Stop", NULL, StopCallback);

            CLAY({
                     .id = CLAY_ID("TopBarSpectrum"),
                     .layout = {
                        .sizing = {CLAY_SIZING_FIXED(200), CLAY_SIZING_GROW()},
                        .padding = CLAY_PADDING_ALL(STANDARD_PADDING),
                        .childGap = STANDARD_GAP,
                        .layoutDirection = CLAY_LEFT_TO_RIGHT,
                        .childAlignment = {CLAY_ALIGN_X_CENTER, CLAY_ALIGN_Y_CENTER}
                },
                     .backgroundColor = COLOR_SCHEME_BACKGROUND_DARK,
                     .cornerRadius = CLAY_CORNER_RADIUS(STANDARD_CORNER_RADIUS)
                 }){

                float* corrected = fifo_audio_to_normal(audio_state.audio_buffers.big_fifo_buffer, frame_arena);
                float* mono = mono_from_stereo(corrected, BIG_FIFO_BUFFER_SIZE, 2, frame_arena); // Going from 2 channels to 1

                SpectrumVisualizer(mono, BIG_FIFO_BUFFER_SIZE);
            }

            CLAY({
                     .id = CLAY_ID("TopBarWaveform"),
                     .layout = {
                        .sizing = {CLAY_SIZING_FIXED(200), CLAY_SIZING_GROW()},
                        .padding = CLAY_PADDING_ALL(STANDARD_PADDING),
                        .childGap = STANDARD_GAP,
                        .layoutDirection = CLAY_LEFT_TO_RIGHT,
                        .childAlignment = {CLAY_ALIGN_X_CENTER, CLAY_ALIGN_Y_CENTER}
                },
                     .backgroundColor = COLOR_SCHEME_BACKGROUND_DARK,
                     .cornerRadius = CLAY_CORNER_RADIUS(STANDARD_CORNER_RADIUS)
                 }){

                float* corrected = fifo_audio_to_normal(audio_state.audio_buffers.fifo_buffer, frame_arena);
                float* mono = mono_from_stereo(corrected, SMALL_FIFO_BUFFER_SIZE, 2, frame_arena); // Going from 2 channels to 1 (2, because thats mono)

                WaveformVisualizer(mono, SMALL_FIFO_BUFFER_SIZE);
            }
        }

        CLAY({
                 .id = CLAY_ID("TopBarStats"), .layout = {.sizing = {CLAY_SIZING_FIXED(300),
                                                                     CLAY_SIZING_GROW()}, .padding = CLAY_PADDING_ALL(0),
                    .layoutDirection = CLAY_TOP_TO_BOTTOM, .childAlignment = {CLAY_ALIGN_X_LEFT, CLAY_ALIGN_Y_CENTER}}
             }) {

            STRING stats = GetTopbarStats();

            CLAY_TEXT(GetString(stats.data), CLAY_TEXT_CONFIG(
                    {.textColor = COLOR_SCHEME_TEXT, .fontSize = 16, .letterSpacing = 0, .wrapMode = CLAY_TEXT_WRAP_NONE}));
        }
    }
}

void generator_edit(int i, Generator generator) {
    CLAY({
         .id = CLAY_IDI("Generator", i),
         .layout = {
         .sizing = {CLAY_SIZING_FIXED(400), CLAY_SIZING_GROW()},
         .padding = CLAY_PADDING_ALL(STANDARD_PADDING),
         .childGap = STANDARD_GAP,
         .layoutDirection = CLAY_TOP_TO_BOTTOM
         },
         .backgroundColor = COLOR_SCHEME_BACKGROUND_TERTIARY,
         }) {
        CLAY({
             .id = CLAY_ID_LOCAL("GeneratorName"),
             .layout = {
             .sizing = {CLAY_SIZING_GROW(), CLAY_SIZING_GROW()},
             .padding = CLAY_PADDING_ALL(STANDARD_PADDING),
             .layoutDirection = CLAY_TOP_TO_BOTTOM,
             .childAlignment = {CLAY_ALIGN_X_LEFT, CLAY_ALIGN_Y_TOP}
             }
             }){

            STRING generatorName = StringCreate("Generator ", frame_arena);
            char *generatorIndex = arena_alloc(frame_arena, 10);
            snprintf(generatorIndex, 10, "%d", i);
            generatorName = StringConcat(&generatorName, generatorIndex);
            CLAY_TEXT(GetString(generatorName.data), CLAY_TEXT_CONFIG(
                          {.textColor = COLOR_SCHEME_TEXT, .fontSize = 16, .letterSpacing = 0, .wrapMode = CLAY_TEXT_WRAP_NONE}));

            STRING generatorType = StringCreate(generator.waveform == SINE ? "Sine" : generator.waveform == SQUARE ? "Square" : generator.waveform == SAWTOOTH ? "Sawtooth" : generator.waveform == TRIANGLE ? "Triangle" : "Noise", frame_arena);
            CLAY_TEXT(GetString(generatorType.data), CLAY_TEXT_CONFIG(
                          {.textColor = COLOR_SCHEME_TEXT, .fontSize = 16, .letterSpacing = 0, .wrapMode = CLAY_TEXT_WRAP_NONE}));


            STRING panning = StringCreate("Pan [-1, 1]: ", frame_arena);
            char *panningString = arena_alloc(frame_arena, 10);
            // 0.00 format
            snprintf(panningString, 10, "%.2f", generator.panning);
            panning = StringConcat(&panning, panningString);

            STRING amplitude = StringCreate("Vol [-1, 1]: ", frame_arena);
            char *amplitudeString = arena_alloc(frame_arena, 10);
            // 0.00 format
            snprintf(amplitudeString, 10, "%.2f", generator.amplitude);
            amplitude = StringConcat(&amplitude, amplitudeString);

            // merge frequency, panning, amplitude into one string
            STRING generatorStats = StringCreate("", frame_arena);
            generatorStats = StringConcat(&generatorStats, panning.data);
            generatorStats = StringConcat(&generatorStats, " / ");
            generatorStats = StringConcat(&generatorStats, amplitude.data);

            CLAY_TEXT(GetString(generatorStats.data), CLAY_TEXT_CONFIG(
                {.textColor = COLOR_SCHEME_TEXT, .fontSize = 16, .letterSpacing = 0, .wrapMode = CLAY_TEXT_WRAP_WORDS}));

            STRING adsr = StringCreate("ADSR: ", frame_arena);
            CLAY_TEXT(GetString(adsr.data), CLAY_TEXT_CONFIG(
                          {.textColor = COLOR_SCHEME_TEXT, .fontSize = 16, .letterSpacing = 0, .wrapMode = CLAY_TEXT_WRAP_NONE}));

            CLAY({
                     .id = CLAY_IDI("ADSR", i),
                     .layout = {
                        .sizing = {CLAY_SIZING_GROW(), CLAY_SIZING_GROW()},
                        .padding = CLAY_PADDING_ALL(STANDARD_PADDING),
                        .childGap = STANDARD_GAP,
                        .layoutDirection = CLAY_LEFT_TO_RIGHT,
                        .childAlignment = {CLAY_ALIGN_X_CENTER, CLAY_ALIGN_Y_CENTER}
                },
                     .backgroundColor = COLOR_SCHEME_BACKGROUND_DARK,
                     .cornerRadius = CLAY_CORNER_RADIUS(STANDARD_CORNER_RADIUS)
                 }) {
                AdsrEnvelope env = generator.envelope;
                AdsrVisualizer(env);
            }
        }
    }
}

void BottomBar(){
    CLAY({
             .id = CLAY_ID("BottomBar"),
             .layout = {
                .sizing = {CLAY_SIZING_GROW(), CLAY_SIZING_PERCENT(0.20)},
                .padding = CLAY_PADDING_ALL(STANDARD_PADDING),
                .childGap = STANDARD_GAP,
                .layoutDirection = CLAY_LEFT_TO_RIGHT,
        },
             .backgroundColor = COLOR_SCHEME_BACKGROUND_SECONDARY,
             .cornerRadius = CLAY_CORNER_RADIUS(STANDARD_CORNER_RADIUS),
             .scroll = {true, false} // horizontal scroll
         }) {

        for(int i = 0; i < audio_state.generator_state.generatorCount; i++){
            Generator generator = audio_state.generator_state.generators[i];
            generator_edit(i, generator);
        }
    }
}
void Root(){
    CLAY({
             .id = CLAY_ID("RootContainer"),
             .layout = {
                .sizing = {CLAY_SIZING_GROW(), CLAY_SIZING_GROW()},
                .padding = CLAY_PADDING_ALL(STANDARD_PADDING * 2),
                .childGap = STANDARD_GAP * 2,
                .layoutDirection = CLAY_TOP_TO_BOTTOM
        },
             .backgroundColor = COLOR_SCHEME_BACKGROUND,
             .cornerRadius = CLAY_CORNER_RADIUS(0)
         }) {
        TopBar();
        BaseContainer();
        BottomBar();
    }
}

void RenderMainUI(void) {
    Root();
}
