#ifndef AUDIO_DEFINITIONS_H
#define AUDIO_DEFINITIONS_H

#define NOTE_C 0
#define NOTE_C_SHARP 1
#define NOTE_D 2
#define NOTE_D_SHARP 3
#define NOTE_E 4
#define NOTE_F 5
#define NOTE_F_SHARP 6
#define NOTE_G 7
#define NOTE_G_SHARP 8
#define NOTE_A 9
#define NOTE_A_SHARP 10
#define NOTE_B 11

static inline int note_to_midi(int note, int octave) {
    return note + octave * 12;
}

#endif // AUDIO_DEFINITIONS_H