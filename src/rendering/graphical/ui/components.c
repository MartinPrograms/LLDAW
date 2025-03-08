#include <string.h>
#include "components.h"
#include "ui_settings.h"
#include "custom.h"
#include "../../../helpers/include.h"
#include "../../../helpers/basic/definitions.h"

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

void AdsrVisualizer(AdsrEnvelope envelope) {
    // black background, green lines
    CustomComponentIndex++;

    CLAY({
             .id = CLAY_IDI("AdsrVisualizer", CustomComponentIndex),
             .layout = {
                .sizing = {CLAY_SIZING_GROW(), CLAY_SIZING_GROW()}, // the user is responsible for sizing the component
                .padding = CLAY_PADDING_ALL(0),
                .childGap = 0,
                .layoutDirection = CLAY_TOP_TO_BOTTOM
        },
             .cornerRadius = CLAY_CORNER_RADIUS(0)
         }) {

        // The actual custom component
        CustomElement adsrElement = {
            .type = CUSTOM_ELEMENT_TYPE_ADSR,
            .color = COLOR_SCHEME_WAVEFORM,
            .customData.adsr = {
                .envelope = envelope
            }
        };

        void* ptr = arena_alloc(frame_arena, sizeof(CustomElement));
        memcpy(ptr, &adsrElement, sizeof(CustomElement));

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


void HandleButtonInteraction(__unused Clay_ElementId id, Clay_PointerData data, intptr_t userData){
    ButtonData *buttonData = (ButtonData *)userData;
    void (*callback)(void*) = buttonData->callback;
    void *user_data = buttonData->userData;
    bool useUserData = buttonData->useUserData;

    if (data.state == CLAY_POINTER_DATA_PRESSED){
        if (useUserData){
            callback(user_data);
        } else {
            callback(NULL);
        }
    }
}

/// Accepts a lambda function as a callback
int ButtonCount = 0;
void CreateButton(const char* text, void* userData, void (*callback)(void*)) {
    ButtonCount++;
    Clay_ElementId id = CLAY_IDI("BUTTON", ButtonCount);
    CLAY({
             .id = (id), .layout = {.sizing = {CLAY_SIZING_FIT(),
                                               CLAY_SIZING_FIT()}, .padding = CLAY_PADDING_ALL(STANDARD_PADDING)},
             .backgroundColor = COLOR_SCHEME_BUTTON,
             .cornerRadius = CLAY_CORNER_RADIUS(STANDARD_CORNER_RADIUS)
         }) {
        ButtonData *dataPtr = arena_alloc(frame_arena, sizeof(ButtonData));
        dataPtr->text = text;
        dataPtr->userData = userData;
        dataPtr->callback = callback;
        dataPtr->useUserData = userData != NULL;
        Clay_OnHover(HandleButtonInteraction, (intptr_t)dataPtr);
        CLAY({
                 .id = CLAY_ID_LOCAL("ButtonText"), .layout = {.sizing = {CLAY_SIZING_GROW(),
                                                                          CLAY_SIZING_GROW()},
                    .padding = CLAY_PADDING_ALL(STANDARD_PADDING)},
             }) {
            CLAY_TEXT(GetString(text), CLAY_TEXT_CONFIG(
                    {.textColor = COLOR_SCHEME_TEXT, .fontSize = 16, .letterSpacing = 0, .wrapMode = CLAY_TEXT_WRAP_NONE}));
        }
    }
}

Clay_String GetString(const char *string) {
    Clay_String result;
    result = (CLAY__INIT(Clay_String){.length=strlen(string), .chars=(string)});
    return result;
}

void CreateDropdown(int id, const char **items, int itemCount, int selectedIndex, int *outIndex) {
    // First we need to create a state for the dropdown
    DropdownState *state = arena_alloc(frame_arena, sizeof(DropdownState));
    state->isOpen = false;
    state->selectedIndex = selectedIndex;
    state->items = items;

    // Now we create the dropdown
    CLAY({
             .id = CLAY_IDI("Dropdown", id),
             .layout = {
                .sizing = {CLAY_SIZING_FIT(), CLAY_SIZING_FIT()},
                .padding = CLAY_PADDING_ALL(STANDARD_PADDING),
                .layoutDirection = CLAY_TOP_TO_BOTTOM
        },
             .backgroundColor = COLOR_SCHEME_BACKGROUND_SECONDARY,
             .cornerRadius = CLAY_CORNER_RADIUS(STANDARD_CORNER_RADIUS)
         }) {
        // The dropdown button
        CLAY({
                 .id = CLAY_ID_LOCAL("DropdownButton"),
                 .layout = {
                    .sizing = {CLAY_SIZING_FIT(), CLAY_SIZING_FIT()},
                    .padding = CLAY_PADDING_ALL(STANDARD_PADDING),
                    .layoutDirection = CLAY_LEFT_TO_RIGHT
            },
                 .backgroundColor = COLOR_SCHEME_BUTTON,
                 .cornerRadius = CLAY_CORNER_RADIUS(STANDARD_CORNER_RADIUS)
             }) {
            ButtonData *dataPtr = arena_alloc(frame_arena, sizeof(ButtonData));
            dataPtr->text = items[selectedIndex];
            dataPtr->userData = state;
            dataPtr->callback = NULL;
            dataPtr->useUserData = true;
            Clay_OnHover(HandleButtonInteraction, (intptr_t) dataPtr);
            CLAY({
                     .id = CLAY_ID_LOCAL("DropdownButtonText"),
                     .layout = {
                        .sizing = {CLAY_SIZING_GROW(), CLAY_SIZING_GROW()},
                        .padding = CLAY_PADDING_ALL(STANDARD_PADDING),
                        .layoutDirection = CLAY_LEFT_TO_RIGHT
                },
                 }) {
                CLAY_TEXT(GetString(items[selectedIndex]), CLAY_TEXT_CONFIG(
                        {.textColor = COLOR_SCHEME_TEXT, .fontSize = 16, .letterSpacing = 0, .wrapMode = CLAY_TEXT_WRAP_NONE}));
            }
        }

        // The dropdown menu
        if (state->isOpen) {
            CLAY({
                     .id = CLAY_ID_LOCAL("DropdownMenu"),
                     .layout = {
                        .sizing = {CLAY_SIZING_FIT(), CLAY_SIZING_FIT()},
                        .padding = CLAY_PADDING_ALL(STANDARD_PADDING),
                        .layoutDirection = CLAY_TOP_TO_BOTTOM
                },
                     .backgroundColor = COLOR_SCHEME_BACKGROUND,
                     .cornerRadius = CLAY_CORNER_RADIUS(STANDARD_CORNER_RADIUS)
                 }) {
                for (int i = 0; i < itemCount; i++) {
                    CLAY({
                             .id = CLAY_IDI("DropdownItem", i),
                             .layout = {
                                .sizing = {CLAY_SIZING_FIT(), CLAY_SIZING_FIT()},
                                .padding = CLAY_PADDING_ALL(STANDARD_PADDING),
                                .layoutDirection = CLAY_LEFT_TO_RIGHT
                        },
                             .backgroundColor = COLOR_SCHEME_BUTTON,
                             .cornerRadius = CLAY_CORNER_RADIUS(STANDARD_CORNER_RADIUS)
                         }) {
                        ButtonData *dataPtr = arena_alloc(frame_arena, sizeof(ButtonData));
                        dataPtr->text = items[i];
                        dataPtr->userData = state;
                        dataPtr->callback = NULL;
                        dataPtr->useUserData = true;
                        Clay_OnHover(HandleButtonInteraction, (intptr_t) dataPtr);
                        CLAY({
                                 .id = CLAY_ID_LOCAL("DropdownItemText"),
                                 .layout = {
                                    .sizing = {CLAY_SIZING_GROW(), CLAY_SIZING_GROW()},
                                    .padding = CLAY_PADDING_ALL(STANDARD_PADDING),
                                    .layoutDirection = CLAY_LEFT_TO_RIGHT
                            },
                             }) {
                            CLAY_TEXT(GetString(items[i]), CLAY_TEXT_CONFIG(
                                    {.textColor = COLOR_SCHEME_TEXT, .fontSize = 16, .letterSpacing = 0, .wrapMode = CLAY_TEXT_WRAP_NONE}));
                        }
                    }
                }
            }
        }
    }

    *outIndex = state->selectedIndex;
}

void Reset() {
    CustomComponentIndex = 0;
    ButtonCount = 0;
}

