#include <string.h>
#include "components.h"
#include "ui_settings.h"
#include "custom.h"
#include "../../../helpers/include.h"

int CustomComponentIndex = 0;
void WaveformVisualizer(const float *buffer, int bufferSize) {
    CustomComponentIndex++;
    CLAY({
             .id = CLAY_IDI("WaveformVisualizer", CustomComponentIndex),
             .layout = {
                .sizing = {CLAY_SIZING_GROW(), CLAY_SIZING_GROW()}, // the user is responsible for sizing the component
                .padding = CLAY_PADDING_ALL(0),
                .childGap = 0,
                .layoutDirection = CLAY_TOP_TO_BOTTOM
        },
             .cornerRadius = CLAY_CORNER_RADIUS(0)
         }) {

        // The actual custom component
        CustomElement waveformElement = {
            .type = CUSTOM_ELEMENT_TYPE_WAVEFORM,
            .color = COLOR_SCHEME_WAVEFORM,
            .customData.waveform = {
                .buffer = buffer,
                .bufferSize = bufferSize
            }
        };

        void* ptr = arena_alloc(frame_arena, sizeof(CustomElement));
        memcpy(ptr, &waveformElement, sizeof(CustomElement));

        CLAY({
            .layout = {
                .sizing = {CLAY_SIZING_GROW(), CLAY_SIZING_GROW()},
                .padding = CLAY_PADDING_ALL(0),
                .childGap = 0,
                .layoutDirection = CLAY_TOP_TO_BOTTOM
            },
            .custom = {
                    .customData = ptr,
            }
        });
    }
}

void SpectrumVisualizer(const float *buffer, int bufferSize) {
    CustomComponentIndex++;
    CLAY({
             .id = CLAY_IDI("SpectrumVisualizer", CustomComponentIndex),
             .layout = {
                .sizing = {CLAY_SIZING_GROW(), CLAY_SIZING_GROW()}, // the user is responsible for sizing the component
                .padding = CLAY_PADDING_ALL(0),
                .childGap = 0,
                .layoutDirection = CLAY_TOP_TO_BOTTOM
        },
             .cornerRadius = CLAY_CORNER_RADIUS(0)
         }) {

        // The actual custom component
        CustomElement spectrumElement = {
            .type = CUSTOM_ELEMENT_TYPE_SPECTRUM,
            .color = COLOR_SCHEME_WAVEFORM,
            .customData.spectrum = {
                .buffer = buffer,
                .bufferSize = bufferSize,
                .quality = 4096
            }
        };

        void* ptr = arena_alloc(frame_arena, sizeof(CustomElement));
        memcpy(ptr, &spectrumElement, sizeof(CustomElement));

        CLAY({
            .layout = {
                .sizing = {CLAY_SIZING_GROW(), CLAY_SIZING_GROW()},
                .padding = CLAY_PADDING_ALL(0),
                .childGap = 0,
                .layoutDirection = CLAY_TOP_TO_BOTTOM
            },
            .custom = {
                    .customData = ptr,
            }
        });
    }
}
