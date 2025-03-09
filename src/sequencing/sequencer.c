#include "sequencer.h"

SequencerState sequencer = {0};

void sequencer_init(){
    sequencer.arena = arena_create(MAX_NOTES * sizeof(Note));
    sequencer.bpm = 130.0f;
    sequencer.length = 60 * SAMPLE_RATE; // 60 seconds
    sequencer.notes = (NoteStack) {
        .notes = (Note*)arena_alloc(sequencer.arena, MAX_NOTES * sizeof(Note)),
        .count = 0,
        .maxNotes = MAX_NOTES
    };
}

void sequencer_add_note(Note note){
    sequencer.notes.notes[sequencer.notes.count].idx = sequencer.notes.count;
    sequencer.notes.notes[sequencer.notes.count++] = note;
}

Note * sequencer_get_notes_at(int64_t sample, int *count, ARENA* arena) {
    int ncount = 0;
    int notes[sequencer.notes.count];
    for (int i = 0; i < sequencer.notes.count; i++) {
        if (sequencer.notes.notes[i].start_sample <= sample && sequencer.notes.notes[i].end_sample >= sample) {
            notes[ncount] = i;
            ncount++;
        }
    }

    *count = ncount;

    Note* result = (Note*)arena_alloc(arena, ncount * sizeof(Note));
    for (int i = 0; i < ncount; i++) {
        result[i] = sequencer.notes.notes[notes[i]];
    }

    return result;
}