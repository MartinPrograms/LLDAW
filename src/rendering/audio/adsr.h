#ifndef ADSR_H
#define ADSR_H

#include <stdint.h>
#include "../../helpers/audio/audio_math.h"

typedef struct {
    float value;
    float tension;
} AdsrValue;

typedef struct {
    AdsrValue attack;
    AdsrValue decay;
    float sustain;
    AdsrValue release;
} AdsrEnvelope; // Attack, Decay, Sustain, Release

AdsrEnvelope adsr_envelope_basic();
float adsr_envelope_apply(float value, int64_t current_sample, int64_t start_sample, int64_t end_sample, const AdsrEnvelope envelope, bool ended, float sampleRate, bool* remove);

#endif //ADSR_H
