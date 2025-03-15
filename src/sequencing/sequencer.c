#include "sequencer.h"

SequencerState sequencer = {0};

void sequencer_init(){
    sequencer.arena = arena_create(MAX_NOTES * sizeof(Note));
    sequencer.settings = (SequencerSettings) {
        .bpm = 130,
        .length = 16, // 16 beats
        .tracker = true
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

void sequencer_remove_note(Note note) {
    // Remove the note
    for (int i = 0; i < sequencer.notes.count; i++) {
        if (sequencer.notes.notes[i].midi_note == note.midi_note) {
            for (int j = i; j < sequencer.notes.count - 1; j++) {
                sequencer.notes.notes[j] = sequencer.notes.notes[j + 1];
            }
            sequencer.notes.count--;
            break;
        }
    }
}

Note * sequencer_get_notes_at(int64_t sample, int *count, ARENA* arena) {
    int ncount = 0;
    int notes[sequencer.notes.count];

    for (int i = 0; i < sequencer.notes.count; i++) {
        Note n = sequencer.notes.notes[i];
        float startSample = samples_from_bpm_time(sequencer.settings.bpm, SAMPLE_RATE, n.start_beat);
        float endSample = samples_from_bpm_time(sequencer.settings.bpm, SAMPLE_RATE, n.end_beat);
        sequencer.notes.notes[i].first_sample = startSample;
        sequencer.notes.notes[i].last_sample = endSample -1; // -1 to make sure we don't include the last sample
        if (sample >= startSample && sample < endSample) {
            notes[ncount++] = i;
        }
    }

    *count = ncount;

    Note* result = (Note*)arena_alloc(arena, ncount * sizeof(Note));
    for (int i = 0; i < ncount; i++) {
        result[i] = sequencer.notes.notes[notes[i]];
    }

    return result;
}
