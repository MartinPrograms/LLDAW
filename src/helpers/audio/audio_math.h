#ifndef AUDIO_MATH_H
#define AUDIO_MATH_H

#include <math.h>
#include <tinycthread.h>
#include <stdint.h>
#include <stdlib.h>
#include "../basic/arena.h"
#include "../basic/definitions.h"
#include "../math/common_math.h"

// CPU specific implementations
#if defined(__x86_64__)
#include "../math/avx2.h"
#endif
#include <stdio.h>
#include <time.h>

#include "../math/neon.h"

#ifndef PI
#define PI 3.14159265358979323846f
#endif

#define WAVE_TABLE_SIZE 8192
#define NUM_WAVEFORMS 5

typedef enum {
    WT_SINE,
    WT_SQUARE,
    WT_SAWTOOTH,
    WT_TRIANGLE,
    WT_NOISE
} WaveformType;

extern float wave_tables[NUM_WAVEFORMS][WAVE_TABLE_SIZE]; // low quality, yes, but very fast

void init_math();

// TODO: add neon implementation (not at my mac right now)

static inline float lerp(float a, float b, float t) {
    #if defined(AVX2_H)
        __m256 m256 = lerp_avx2(from_float(a), from_float(b), from_float(t));
        return to_float(m256);
    #endif

    return a + t * (b - a);
}

/// A lerp, with tension parameter to control the curve
static inline float lerp_tension(float a, float b, float t, float tension) {
    #if defined(AVX2_H)
        __m256 m256 = lerp_tension_avx2(_mm256_set1_ps(a), _mm256_set1_ps(b), _mm256_set1_ps(t), _mm256_set1_ps(tension));
        return to_float(m256);
    #endif

    return lerp(a, b, t * (1.0f - tension) + t * t * t * tension);
}

/// Quick lookup (using a precomputed table)
float quick_lookup(float phase, WaveformType waveform);

/// Quick calculate (100% accurate, unlike quick_lookup. Though slower, depending on the waveform)
float quick_calculate(float phase, WaveformType waveform);

static inline float map(float value, float start1, float stop1, float start2, float stop2) {
#if defined(AVX2_H)
    __m256 m256 = map_avx2(from_float(value), from_float(start1), from_float(stop1), from_float(start2), from_float(stop2));
    return to_float(m256);
#endif

    return start2 + (stop2 - start2) * ((value - start1) / (stop1 - start1));
}

static inline float clamp(float value, float min, float max) {
#if defined(AVX2_H)
    __m256 m256 = clamp_avx2(from_float(value), from_float(min), from_float(max));
    return to_float(m256);
#endif

    return fmin(fmax(value, min), max);
}

static inline float smoothstep(float edge0, float edge1, float x) {
#if defined(AVX2_H)
    __m256 m256 = smoothstep_avx2(from_float(edge0), from_float(edge1), from_float(x));
    return to_float(m256);
#endif

    x = clamp((x - edge0) / (edge1 - edge0), 0.0f, 1.0f);
    return x * x * (3 - 2 * x);
}

static inline float panning(float value, float pan, bool rightChannel) {
#if defined(AVX2_H)
    __m256 m256 = panning_avx2(from_float(value), from_float(pan), (rightChannel));
    return to_float(m256);
#endif

    float result = value;

    if (!rightChannel) {
        result *= (pan + 1.0f) * 0.5f;
    } else {
        result *=  (1.0f - pan) * 0.5f;
    }

    return result;
}

static inline float gain(float value, float gain) {
#if defined(AVX2_H)
    __m256 m256 = from_float(value);
    m256 = _mm256_mul_ps(m256, from_float(gain));
    return to_float(m256);
#endif
    return value * gain;
}

/// Channel [0,1,2] -> [left, right, mono (combined)]
static inline float* mono_from_stereo(float* stereo, int size, int channel, ARENA* arena) {
#if defined(AVX2_H)
    float* output = mono_from_stereo_avx2(stereo, size, channel, arena);
    return output;
#endif

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

// Will NOT be converted to AVX2, as it's not performance critical
static inline float midi_note_to_frequency(int note) {
    return 440.0f * powf(2.0f, (note - 69) / 12.0f); // TODO: add tuning
}

// Will NOT be converted to AVX2, as it's not performance critical. And should only be used sparingly, and for sampling we have a rand function that uses a lookup table
static inline float random_value(float min, float max) {
    return min + (max - min) * ((float)rand() / (float)RAND_MAX);
}

/// Returns the current time in nanoseconds
static inline int64_t get_time_now() { // sure i guess this isnt audio but whatever
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000000000 + ts.tv_nsec;
}

static inline float nanoseconds_to_milliseconds(int64_t nanoseconds) {
    // Convert first to microseconds, then to milliseconds
    return (float)nanoseconds / 1000000.0f;
}

#endif