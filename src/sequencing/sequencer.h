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
    int64_t start_sample;
    int64_t end_sample;
    bool active;
    int idx;
} Note;

typedef struct {
    Note* notes;
    int count;
    int maxNotes;
} NoteStack;

typedef struct {
    NoteStack notes;
    float bpm;
    int64_t length;
    ARENA* arena;
} SequencerState;

extern SequencerState sequencer;

void sequencer_init();
void sequencer_add_note(Note note);

Note * sequencer_get_notes_at(int64_t sample, int *count, ARENA* arena);

#endif // SEQUENCER_H