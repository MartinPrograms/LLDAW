#include "tracker_gui.h"

void tracker_gui(SequencerState *sequencer) {
    // For each generator, draw a new row of notes.
    // First we get the notes for each generator
    CLAY({
         .id = CLAY_ID("GeneratorsNoteDisplay"),
         .layout = {
         .sizing = {CLAY_SIZING_GROW(), CLAY_SIZING_GROW()},
         .padding = CLAY_PADDING_ALL(STANDARD_PADDING),
         .childGap = STANDARD_GAP,
         .layoutDirection = CLAY_LEFT_TO_RIGHT,
         },
         .backgroundColor = COLOR_SCHEME_BACKGROUND_TERTIARY,
         .cornerRadius = CLAY_CORNER_RADIUS(STANDARD_CORNER_RADIUS),
         .scroll = {true, true}
         }) {
        int total_cells = sequencer->settings.length * 16; // 16 cells per beat (sixteenth notes)

        for (int i = 0; i < audio_state.generator_state.generatorCount; i++) {
            CLAY({
                 .id = CLAY_IDI("TrackerGenerator", i),
                 .layout = {
                 .sizing = {CLAY_SIZING_FIT(120), CLAY_SIZING_FIT(0)},
                 .padding = CLAY_PADDING_ALL(STANDARD_PADDING),
                 .childGap = STANDARD_GAP,
                 .layoutDirection = CLAY_TOP_TO_BOTTOM,
                 },
                 .backgroundColor = COLOR_SCHEME_BACKGROUND_SECONDARY,
                 .cornerRadius = CLAY_CORNER_RADIUS(STANDARD_CORNER_RADIUS)
                 }) {
                STRING str = StringCreate("Generator - ", frame_arena);
                STRING intStr = StringFromInt(i, frame_arena);
                str = StringConcatString(&str, &intStr);

                CLAY_TEXT(GetString(str.data), CLAY_TEXT_CONFIG(
                        {.textColor = COLOR_SCHEME_TEXT, .fontSize = 16, .letterSpacing = 0, .wrapMode = CLAY_TEXT_WRAP_NONE}));

                for (int cell = 0; cell < total_cells; cell++) {
                    float current_beat = cell / 16.0f;
                    Note *existing_note = NULL;
                    __unused int existing_note_idx = -1;

                    for (int j = 0; j < sequencer->notes.count; j++) {
                        Note *n = &sequencer->notes.notes[j];
                        if (n->generator_index == i &&
                            n->start_beat <= current_beat &&
                            n->end_beat > current_beat) {
                            existing_note = n;
                            existing_note_idx = j;
                            break;
                        }
                    }

                    int new_note = existing_note ? existing_note->midi_note : 0;
                    int new_vel = existing_note ? (int) (existing_note->amplitude * 127) : 0;
                    bool active = audio_state.current_beat >= current_beat &&
                                  audio_state.current_beat < (cell + 1) / 16.0f;

                    TrackerMidiNote(
                        new_note, new_vel,
                        &new_note, &new_vel,
                        active, i, cell
                    );

                    if ((new_note != (existing_note ? existing_note->midi_note : 0)) ||
                        (new_vel != (existing_note ? (int)(existing_note->amplitude * 127) : 0))) {

                        // Delete existing voices
                        if (existing_note) {
                            generator_kill_voice(existing_note->midi_note,
                                              &audio_state.generator_state.generators[i]);
                        }

                        // Handle zero velocity (note truncation)
                        if (new_vel == 0) {
                            if (existing_note) {
                                // Split the note if we're in the middle
                                if (existing_note->start_beat < current_beat) {
                                    // Truncate existing note
                                    existing_note->end_beat = current_beat;
                                } else {
                                    // Remove note entirely
                                    sequencer_remove_note(sequencer->notes.notes[existing_note_idx]);
                                }
                            }
                        }
                        // Handle non-zero velocity (note creation/modification)
                        else {
                            // Split existing overlapping notes
                            for (int j = 0; j < sequencer->notes.count; j++) {
                                Note *n = &sequencer->notes.notes[j];
                                if (n->generator_index == i &&
                                    n->start_beat < current_beat &&
                                    n->end_beat > current_beat) {
                                    // Split into left segment
                                    Note left = *n;
                                    left.end_beat = current_beat;
                                    sequencer->notes.notes[j] = left;

                                    // Create right segment
                                    Note right = *n;
                                    right.start_beat = current_beat;
                                    sequencer_add_note(right);
                                }
                            }

                            // Create/move note
                            if (existing_note) {
                                existing_note->midi_note = new_note;
                                existing_note->amplitude = new_vel / 127.0f;
                                existing_note->frequency = midi_note_to_frequency(new_note);
                            } else {
                                Note new_note_struct = {
                                    .generator_index = i,
                                    .start_beat = current_beat,
                                    .end_beat = sequencer->settings.length,
                                    .midi_note = new_note,
                                    .amplitude = new_vel / 127.0f,
                                    .frequency = midi_note_to_frequency(new_note)
                                };

                                // Find next note to set proper end_beat
                                for (int j = 0; j < sequencer->notes.count; j++) {
                                    Note *n = &sequencer->notes.notes[j];
                                    if (n->generator_index == i &&
                                        n->start_beat > current_beat) {
                                        new_note_struct.end_beat = n->start_beat;
                                        break;
                                    }
                                }

                                sequencer_add_note(new_note_struct);
                            }
                        }
                    }
                }
            }
        }
    }
}
