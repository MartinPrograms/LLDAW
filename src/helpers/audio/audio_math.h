#ifndef AUDIO_MATH_H
#define AUDIO_MATH_H

#include <math.h>

static inline float lerp(float a, float b, float t) {
    return a + t * (b - a);
}

static inline float map(float value, float start1, float stop1, float start2, float stop2) {
    return start2 + (stop2 - start2) * ((value - start1) / (stop1 - start1));
}

static inline float clamp(float value, float min, float max) {
    return fmin(fmax(value, min), max);
}

static inline float smoothstep(float edge0, float edge1, float x) {
    x = clamp((x - edge0) / (edge1 - edge0), 0.0f, 1.0f);
    return x * x * (3 - 2 * x);
}

static inline float panning(float value, float pan, bool rightChannel) {
    float result = value;

    if (!rightChannel) {
        result *= (pan + 1.0f) * 0.5f;
    } else {
        result *=  (1.0f - pan) * 0.5f;
    }

    return result;
}

static inline float gain(float value, float gain) {
    return value * gain;
}

static inline float adsr(float value, float attack, float decay, float sustain, float release, float sample, float sampleRate) {
    float envelope = 0.0f;
    float timeInSeconds = sample / sampleRate;

    if (timeInSeconds < attack) {
        envelope = lerp(0.0f, 1.0f, timeInSeconds / attack);
    } else if (timeInSeconds < attack + decay) {
        envelope = lerp(1.0f, sustain, (timeInSeconds - attack) / decay);
    } else if (timeInSeconds < attack + decay + sustain) {
        envelope = sustain;
    } else {
        envelope = lerp(sustain, 0.0f, (timeInSeconds - attack - decay - sustain) / release);
    }

    return value * envelope;
}

/// Channel [0,1,2] -> [left, right, mono (combined)]
static inline float* mono_from_stereo(float* stereo, int size, int channel, ARENA* arena) {
    float* mono = (float*)arena_alloc(arena, size * sizeof(float));
    for (int i = 0; i < size; i++) {
        if (channel == 0) {
            mono[i] = stereo[i * 2];
        } else if (channel == 1) {
            mono[i] = stereo[i * 2 + 1];
        } else {
            mono[i] = (stereo[i * 2] + stereo[i * 2 + 1]) / 2.0f;
        }
    }
    return mono;
}

/// Function to return time (in seconds) from samples, given the sample rate
static inline float time_from_samples(int64_t samples, float sampleRate) {
    return (float)samples / sampleRate;
}

/// Function to return samples from time (in seconds), given the sample rate
static inline int64_t samples_from_time(float time, float sampleRate) {
    return (int64_t)(time * sampleRate);
}

static inline float midi_note_to_frequency(int note) {
    return 440.0f * powf(2.0f, (note - 69) / 12.0f); // TODO: add tuning
}

#undef timespec

static inline struct timespec get_time_now() { // sure i guess this isnt audio but whatever
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts;
}

static inline double time_in_seconds(struct timespec ts) { // genuinely dont care atp
    return ts.tv_sec + ts.tv_nsec / 1.0e9;
}

#define timespec _tthread_timespec

#endif