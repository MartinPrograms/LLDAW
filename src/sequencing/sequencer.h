#ifndef SEQUENCER_H
#define SEQUENCER_H

#include <stdint.h>
#include "../helpers/include.h"

#define MAX_NOTES 4096 * 64

typedef struct {
  	int generator_index;
  	float frequency;
    float amplitude;
    float pan;
    float start_beat;
    float end_beat;
    bool active;
    int midi_note;
    int64_t first_sample;
    int64_t last_sample;
} Note;

typedef struct {
    Note* notes;
    int count;
    int maxNotes;
} NoteStack;

typedef struct {
    bool tracker; // If the sequencer is in tracker mode, if so it'll display the UI differently.
    float bpm;
    float length;
} SequencerSettings;

typedef struct {
    NoteStack notes;
    SequencerSettings settings;
    ARENA* arena;
} SequencerState;

extern SequencerState sequencer;

void sequencer_init();
void sequencer_add_note(Note note);
void sequencer_remove_note(Note note);

Note * sequencer_get_notes_at(int64_t sample, int *count, ARENA* arena);

#endif // SEQUENCER_H