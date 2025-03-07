#include "components.h"
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdio.h>
#include <raylib.h>
#include "base_ui.h"

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
                 .id = CLAY_ID("LeftPanelText"), .layout = {.sizing = {CLAY_SIZING_GROW(),
                                                                       CLAY_SIZING_GROW()}, .padding = CLAY_PADDING_ALL(STANDARD_PADDING)},
             }) {
            CLAY_TEXT(GetString("Left Panel"), CLAY_TEXT_CONFIG(
                    {.textColor = COLOR_SCHEME_TEXT, .fontSize = 16, .letterSpacing = 0, .wrapMode = CLAY_TEXT_WRAP_NONE}));
        }
    }
}

void MiddlePanel(){
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
        CLAY({
                 .id = CLAY_ID("MiddlePanelText"), .layout = {.sizing = {CLAY_SIZING_GROW(),
                                                                         CLAY_SIZING_FIT()}, .padding = CLAY_PADDING_ALL(STANDARD_PADDING)},
             }) {
            CLAY_TEXT(GetString("Middle Panel"), CLAY_TEXT_CONFIG(
                    {.textColor = COLOR_SCHEME_TEXT, .fontSize = 16, .letterSpacing = 0, .wrapMode = CLAY_TEXT_WRAP_NONE}));
        }
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
                .padding = CLAY_PADDING_ALL(STANDARD_PADDING),
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
    // Begin long term memory stats (frame arena)
    STRING stats = StringCreate("Long term memory: ", frame_arena);
    float percentageUsed =
            percentage_arena_used(default_arena) * 100; // convert to percentage, and then to string
    char *percentageString = arena_alloc(frame_arena, 10);
    snprintf(percentageString, 10, "%f", percentageUsed);
    stats = StringConcat(&stats, percentageString);
    stats = StringConcat(&stats, "%");
    // End long term memory stats

    // Begin short term memory stats (frame arena)
    stats = StringConcat(&stats, "\n");
    stats = StringConcat(&stats, "Short term memory: ");
    percentageUsed = percentage_arena_used(frame_arena) * 100; // convert to percentage, and then to string
    snprintf(percentageString, 10, "%f", percentageUsed);
    stats = StringConcat(&stats, percentageString);
    stats = StringConcat(&stats, "%");
    // End short term memory stats

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
                                                                    CLAY_SIZING_GROW()}, .padding = CLAY_PADDING_ALL(STANDARD_PADDING), .childAlignment = {CLAY_ALIGN_X_LEFT, CLAY_ALIGN_Y_CENTER}},
             }) {
            CLAY_TEXT(GetString(title.data), CLAY_TEXT_CONFIG(
                    {.textColor = COLOR_SCHEME_TEXT, .fontSize = 32, .letterSpacing = 0, .wrapMode = CLAY_TEXT_WRAP_NONE}));
        }

        CLAY({
                 .id = CLAY_ID("PlaybackControls"), // think play, pause, stop
                 .layout = {
                    .sizing = {CLAY_SIZING_GROW(), CLAY_SIZING_GROW()},
                    .padding = CLAY_PADDING_ALL(0),
                    .childGap = STANDARD_GAP,
                    .layoutDirection = CLAY_LEFT_TO_RIGHT,
                    .childAlignment = {CLAY_ALIGN_X_CENTER, CLAY_ALIGN_Y_CENTER}
            }
             }) {
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
                SpectrumVisualizer(audio_state.bigFifoBuffer, BIG_FIFO_BUFFER_SIZE);
            }

            CreateButton("Play", NULL, PlayCallback);
            CreateButton("Pause", NULL, PauseCallback);
            CreateButton("Stop", NULL, StopCallback);

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

                WaveformVisualizer(audio_state.smallFifoBuffer, SMALL_FIFO_BUFFER_SIZE);
            }
        }

        CLAY({
                 .id = CLAY_ID("TopBarStats"), .layout = {.sizing = {CLAY_SIZING_FIT(),
                                                                     CLAY_SIZING_GROW()}, .padding = CLAY_PADDING_ALL(STANDARD_PADDING),
                    .layoutDirection = CLAY_TOP_TO_BOTTOM},
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
        // Generator name, frequency, amplitude, waveform, with buttons to change them
        CLAY({
             .id = CLAY_ID_LOCAL("GeneratorName"),
             .layout = {
             .sizing = {CLAY_SIZING_GROW(), CLAY_SIZING_FIT()},
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

            STRING frequency = StringCreate("Frequency: ", frame_arena);
            char *frequencyString = arena_alloc(frame_arena, 10);
            // 0.00 format
            snprintf(frequencyString, 10, "%.2f", generator.frequency);
            frequency = StringConcat(&frequency, frequencyString);
            frequency = StringConcat(&frequency, "hz");
            CLAY_TEXT(GetString(frequency.data), CLAY_TEXT_CONFIG(
                          {.textColor = COLOR_SCHEME_TEXT, .fontSize = 16, .letterSpacing = 0, .wrapMode = CLAY_TEXT_WRAP_NONE}));
        }
    }
}

void BottomBar(){
    CLAY({
             .id = CLAY_ID("BottomBar"),
             .layout = {
                .sizing = {CLAY_SIZING_GROW(), CLAY_SIZING_PERCENT(0.15)},
                .padding = CLAY_PADDING_ALL(STANDARD_PADDING),
                .childGap = STANDARD_GAP,
                .layoutDirection = CLAY_LEFT_TO_RIGHT,
        },
             .backgroundColor = COLOR_SCHEME_BACKGROUND_SECONDARY,
             .cornerRadius = CLAY_CORNER_RADIUS(STANDARD_CORNER_RADIUS),
             .scroll = {true, false} // horizontal scroll
         }) {

        for(int i = 0; i < audio_state.generatorState.generatorCount; i++){
            Generator generator = audio_state.generatorState.generators[i];
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
