#ifndef AVX2_H
#define AVX2_H

// X64 AVX2 implementation of the audio math functions

// Only compile this on x64
#if defined(__x86_64__)
#include <stdint.h>
#include <stdio.h>
#include <immintrin.h>
#include "../basic/arena.h"

// Helper functions:

static inline __m256 from_float(float value) {
    return _mm256_set1_ps(value);
}

// from int64
static inline __m256i from_int64(int64_t value) {
    return _mm256_set1_epi64x(value);
}

static inline int64_t to_int64(__m256i v) {
    int64_t tmp[4];
    _mm256_store_si256((__m256i*)tmp, v);
    return tmp[0];
}

static inline float to_float(__m256 v) {
    float tmp[8];
    _mm256_store_ps(tmp, v);
    return tmp[0];
}

// Actual math functions

static inline __m256 lerp_avx2(__m256 a, __m256 b, __m256 t) {
    return _mm256_add_ps(a, _mm256_mul_ps(t, _mm256_sub_ps(b, a)));
}

static inline __m256 lerp_tension_avx2(__m256 a, __m256 b, __m256 t, __m256 tension) {
    __m256 one = _mm256_set1_ps(1.0f);
    __m256 t1 = _mm256_mul_ps(t, _mm256_sub_ps(one, tension));
    __m256 t2 = _mm256_mul_ps(t, _mm256_mul_ps(t, _mm256_mul_ps(t, tension)));
    return lerp_avx2(a, b, _mm256_add_ps(t1, t2));
}

static inline __m256 map_avx2(__m256 value, __m256 start1, __m256 stop1, __m256 start2, __m256 stop2) {
    return _mm256_add_ps(start2, _mm256_mul_ps(_mm256_sub_ps(stop2, start2), _mm256_div_ps(_mm256_sub_ps(value, start1), _mm256_sub_ps(stop1, start1))));
}

static inline __m256 clamp_avx2(__m256 value, __m256 min, __m256 max) {
    return _mm256_min_ps(_mm256_max_ps(value, min), max);
}

static inline __m256 smoothstep_avx2(__m256 edge0, __m256 edge1, __m256 x) {
    __m256 zero = _mm256_set1_ps(0.0f);
    __m256 one = _mm256_set1_ps(1.0f);
    x = clamp_avx2(_mm256_div_ps(_mm256_sub_ps(x, edge0), _mm256_sub_ps(edge1, edge0)), zero, one);
    return _mm256_mul_ps(_mm256_mul_ps(x, x), _mm256_sub_ps(_mm256_set1_ps(3.0f), _mm256_mul_ps(_mm256_set1_ps(2.0f), x)));
}

static inline __m256 panning_avx2(__m256 value, __m256 pan, bool rightChannel) {
    const __m256 one = _mm256_set1_ps(1.0f);
    const __m256 half = _mm256_set1_ps(0.5f);

    // Calculate both channels simultaneously
    const __m256 left_gain = _mm256_mul_ps(_mm256_add_ps(pan, one), half);
    const __m256 right_gain = _mm256_mul_ps(_mm256_sub_ps(one, pan), half);

    // Broadcast selection mask
    const __m256 result = rightChannel
        ? _mm256_mul_ps(value, right_gain)
        : _mm256_mul_ps(value, left_gain);

    return result;
}

static inline float* mono_from_stereo_avx2(float* stereo, int size, int channel, ARENA* arena) {
    const __m256i left_indices = _mm256_setr_epi32(0, 2, 4, 6, 8, 10, 12, 14);
    const __m256i right_indices = _mm256_setr_epi32(1, 3, 5, 7, 9, 11, 13, 15);
    const __m256 half = _mm256_set1_ps(0.5f);

    float* mono = (float*)arena_alloc(arena, size * sizeof(float));
    int i = 0;

    for (; i <= size - 8; i += 8) {
        const int stereo_base = i * 2;
        const __m256i base = _mm256_set1_epi32(stereo_base);

        __m256 data;
        switch(channel) {
            case 0: {  // Left channel
                const __m256i indices = _mm256_add_epi32(base, left_indices);
                data = _mm256_i32gather_ps(stereo, indices, 4);
                break;
            }
            case 1: {  // Right channel
                const __m256i indices = _mm256_add_epi32(base, right_indices);
                data = _mm256_i32gather_ps(stereo, indices, 4);
                break;
            }
            default: { // Average
                const __m256i left_idx = _mm256_add_epi32(base, left_indices);
                const __m256i right_idx = _mm256_add_epi32(base, right_indices);
                const __m256 left = _mm256_i32gather_ps(stereo, left_idx, 4);
                const __m256 right = _mm256_i32gather_ps(stereo, right_idx, 4);
                data = _mm256_mul_ps(_mm256_add_ps(left, right), half);
                break;
            }
        }
        _mm256_storeu_ps(mono + i, data);
    }

    return mono;
}

#endif

#endif