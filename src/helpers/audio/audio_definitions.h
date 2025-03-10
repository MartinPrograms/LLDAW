#ifndef AUDIO_DEFINITIONS_H
#define AUDIO_DEFINITIONS_H

#define SAMPLE_RATE 44100
#define BUFFER_SIZE 1024
#define BIG_FIFO_BUFFER_SIZE SAMPLE_RATE / 2
#define SMALL_FIFO_BUFFER_SIZE 1024
#define AUDIO_THREAD_PRIORITY 0 // 0 is considered default priority, if clicking occurs try setting to 1 which raises it to critical
#define AUDIO_BUFFER_COUNT 3

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

#define WHOLE 1.0f
#define HALF 0.5f
#define QUARTER 0.25f
#define EIGHTH 0.125f
#define SIXTEENTH 0.0625f


static inline int note_to_midi(int note, int octave) {
    return note + octave * 12;
}

static inline int64_t samples_from_bpm(float bpm, float sampleRate) {
    return (int64_t)(60.0f / bpm * sampleRate);
}

static inline int64_t samples_from_bpm_time_component(float bpm, float sampleRate, float component) {
    int64_t samples = (int64_t)((float)samples_from_bpm(bpm, sampleRate) * (float)component);
    printf("%lld\n", samples);
    return samples;
}

static inline int64_t samples_from_bpm_time(float bpm, float sampleRate, float time) {
    return samples_from_bpm(bpm, sampleRate) * time;
}

#endif // AUDIO_DEFINITIONS_H