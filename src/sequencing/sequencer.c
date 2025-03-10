#include "sequencer.h"

SequencerState sequencer = {0};

void sequencer_init(){
    sequencer.arena = arena_create(MAX_NOTES * sizeof(Note));
    sequencer.settings = (SequencerSettings) {
        .bpm = 130,
        .length = 16,
        .tracker = false
    };
    sequencer.notes = (NoteStack) {
        .notes = (Note*)arena_alloc(sequencer.arena, MAX_NOTES * sizeof(Note)),
        .count = 0,
        .maxNotes = MAX_NOTES
    };

    // Check if sequencer.notes.notes is NULL, if so we failed to allocate memory
    if (!sequencer.notes.notes) {
        printf("Failed to allocate memory for sequencer notes\n");
        exit(1);
    }
}

void sequencer_add_note(Note note){
    if (sequencer.notes.count >= sequencer.notes.maxNotes) {
        printf("Max notes reached\n");
        return;
    }

    sequencer.notes.notes[sequencer.notes.count].midi_note = sequencer.notes.count;
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