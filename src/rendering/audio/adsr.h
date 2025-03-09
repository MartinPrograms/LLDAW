#ifndef ADSR_H
#define ADSR_H

#include <stdint.h>
#include "../../helpers/audio/audio_math.h"

typedef struct {
    float value;
    float tension;
} AdsrValue;

typedef struct {
    float* active;
    int64_t activeLength;
    int64_t inactiveLength;
} AdsrCache;

typedef struct {
    AdsrValue attack;
    AdsrValue decay;
    float sustain;
    AdsrValue release;
    AdsrCache cache;
} AdsrEnvelope; // Attack, Decay, Sustain, Release

AdsrEnvelope adsr_envelope_basic();
float adsr_envelope_apply(float value, int64_t current_sample, int64_t start_sample, int64_t end_sample, const AdsrEnvelope envelope, bool ended, float sampleRate, bool* remove);
float adsr_from_cache(float value, int64_t current_sample, int64_t start_sample, int64_t end_sample, const AdsrEnvelope envelope, bool ended, bool* remove);
void adsr_cache_envelope(AdsrEnvelope *envelope, ARENA* arena, float sampleRate);
#endif //ADSR_H
