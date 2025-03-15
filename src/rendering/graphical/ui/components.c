#include <string.h>
#include "components.h"
#include "ui_settings.h"
#include "custom.h"
#include "ui_renderer.h"
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

static struct {
    int activeGenerator;
    int activeStep;
    int dragStartY;
    int initialNote;
    bool draggingNote;
    bool draggingVelocity;
} midiDragState = {-1, -1, 0, 0, false, false};

void TrackerMidiNote(int midi_note, int velocity, int *adjusted_midi_note, int *adjusted_midi_velocity, bool active, int generator_index, int cell) {
    CustomComponentIndex++;
    *adjusted_midi_note = midi_note;
    *adjusted_midi_velocity = velocity;
    CLAY({
         .id = CLAY_IDI("TrackerMidiNote", CustomComponentIndex),
         .layout = {
            .sizing = {CLAY_SIZING_GROW(), CLAY_SIZING_GROW()},
            .padding = CLAY_PADDING_ALL(0),
            .childGap = 0,
            .layoutDirection = CLAY_TOP_TO_BOTTOM
         },
         .cornerRadius = CLAY_CORNER_RADIUS(0)
    }) {



        int note_str_len;
        char* note_str = midi_to_str(midi_note, &note_str_len, frame_arena);
        char note_velocity_str[4];
        snprintf(note_velocity_str, 4, "%d", velocity);

        // Now we have the necessary values to draw the note
        // Draw the note name using Clay

        STRING note_name = StringCreate(note_str, frame_arena);
        STRING note_velocity = StringCreate(note_velocity_str, frame_arena);

        bool isActiveNote = midiDragState.draggingNote &&
                           (midiDragState.activeGenerator == generator_index) &&
                           (midiDragState.activeStep == cell);

        Clay_Color background_color = COLOR_SCHEME_BACKGROUND_SECONDARY;
        if (active) background_color = (Clay_Color) COLOR_SCHEME_BACKGROUND_DARK;

        if (Clay_PointerOver(CLAY_IDI("TrackerMidiNote", CustomComponentIndex))) {
            background_color = (Clay_Color)COLOR_SCHEME_BACKGROUND_TERTIARY;

            CustomScrollCapture = true;

            if (GetMouseWheelMove() > 0) {
                *adjusted_midi_note += 1;
                if (*adjusted_midi_note > 127) {
                    *adjusted_midi_note = 127;
                }
            } else if (GetMouseWheelMove() < 0) {
                *adjusted_midi_note -= 1;
                if (*adjusted_midi_note < 0) {
                    *adjusted_midi_note = 0;
                }
            }

            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                // Start drag operation
                midiDragState.activeGenerator = generator_index;
                midiDragState.activeStep = cell;
                midiDragState.dragStartY = GetMouseY();
                midiDragState.initialNote = *adjusted_midi_note;
                midiDragState.draggingNote = true;
            }

            if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
                // Start drag operation
                midiDragState.activeGenerator = generator_index;
                midiDragState.activeStep = cell;
                midiDragState.dragStartY = GetMouseY();
                midiDragState.initialNote = *adjusted_midi_velocity;
                midiDragState.draggingNote = false;
                midiDragState.draggingVelocity = true;
            }
        }
        if (isActiveNote) background_color = (Clay_Color) COLOR_SCHEME_BACKGROUND;

        if ((midiDragState.draggingNote || midiDragState.draggingVelocity) &&
            midiDragState.activeGenerator == generator_index &&
            midiDragState.activeStep == cell)
        {
            int currentY = GetMouseY();
            int deltaY = midiDragState.dragStartY - currentY;
            int deltaNotes = deltaY / 5;  // 5 pixels per note change

            if (deltaNotes != 0) {
                if (midiDragState.draggingNote) {
                    *adjusted_midi_note = midiDragState.initialNote + deltaNotes;
                    // Limit the note to the MIDI range
                    if (*adjusted_midi_note < 0) {
                        *adjusted_midi_note = 0;
                    } else if (*adjusted_midi_note > 127) {
                        *adjusted_midi_note = 127;
                    }
                } else {
                    *adjusted_midi_velocity = midiDragState.initialNote + deltaNotes;
                    // Limit the velocity to the MIDI range
                    if (*adjusted_midi_velocity < 0) {
                        *adjusted_midi_velocity = 0;
                    } else if (*adjusted_midi_velocity > 127) {
                        *adjusted_midi_velocity = 127;
                    }
                }
            }

            // End drag on mouse release
            if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
                midiDragState.draggingNote = false;
                midiDragState.activeGenerator = -1;
                midiDragState.activeStep = -1;

            }

            if (IsMouseButtonReleased(MOUSE_BUTTON_RIGHT)) {
                midiDragState.draggingVelocity = false;
                midiDragState.activeGenerator = -1;
                midiDragState.activeStep = -1;

            }
        }

        // Display the note on the left, and the velocity on the right
        CLAY({
                 .id = CLAY_ID_LOCAL("NoteDisplay"),
                 .layout = {
                    .sizing = {CLAY_SIZING_GROW(), CLAY_SIZING_GROW()},
                    .padding = CLAY_PADDING_ALL(STANDARD_PADDING),
                    .layoutDirection = CLAY_LEFT_TO_RIGHT
             },
                 .backgroundColor = background_color,
                 .cornerRadius = CLAY_CORNER_RADIUS(STANDARD_CORNER_RADIUS)
             }) {

            CLAY ({
                .id = CLAY_ID_LOCAL("Cell"),
                .layout = {
                    .sizing = {CLAY_SIZING_FIT(), CLAY_SIZING_FIT()},
                    .layoutDirection = CLAY_LEFT_TO_RIGHT,
                    .childAlignment = {CLAY_ALIGN_X_LEFT, CLAY_ALIGN_Y_CENTER}
                }
            }) {
                STRING cell_str_nr = StringFromInt(cell, frame_arena);
                CLAY_TEXT(GetString(cell_str_nr.data), CLAY_TEXT_CONFIG(
                        {.textColor = COLOR_SCHEME_TEXT, .fontSize = STANDARD_FONT_SIZE, .letterSpacing = 0, .wrapMode = CLAY_TEXT_WRAP_NONE}));
            }

            CLAY ({
            .id = CLAY_ID_LOCAL("NoteName"),
                .layout = {
                    .sizing = {CLAY_SIZING_GROW(), CLAY_SIZING_FIT()},
                    .layoutDirection = CLAY_LEFT_TO_RIGHT,
                    .childAlignment = {CLAY_ALIGN_X_CENTER, CLAY_ALIGN_Y_CENTER}
                }
            }) {
                CLAY_TEXT(GetString(note_name.data), CLAY_TEXT_CONFIG(
                        {.textColor = COLOR_SCHEME_TEXT, .fontSize = STANDARD_FONT_SIZE, .letterSpacing = 0, .wrapMode = CLAY_TEXT_WRAP_NONE}));
            }

            CLAY({
                .id = CLAY_ID_LOCAL("NoteVelocity"),
                .layout = {
                    .sizing = {CLAY_SIZING_GROW(), CLAY_SIZING_GROW()},
                    .layoutDirection = CLAY_LEFT_TO_RIGHT,
                    .childAlignment = {CLAY_ALIGN_X_RIGHT, CLAY_ALIGN_Y_CENTER}
                }
            }) {
                CLAY_TEXT(GetString(note_velocity.data), CLAY_TEXT_CONFIG(
                        {.textColor = COLOR_SCHEME_TEXT, .fontSize = STANDARD_FONT_SIZE, .letterSpacing = 0, .wrapMode = CLAY_TEXT_WRAP_NONE}));
            }

        }
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

