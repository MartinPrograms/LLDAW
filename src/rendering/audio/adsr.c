#include "adsr.h"

#include "../../helpers/include.h"

float adsr_envelope_apply(float value, int64_t current_sample, int64_t start_sample, int64_t end_sample,
                          const AdsrEnvelope envelope, bool ended, float sampleRate, bool* remove) {
    const int64_t attackInSamples  = samples_from_time(envelope.attack.value, sampleRate);
    const int64_t decayInSamples   = samples_from_time(envelope.decay.value, sampleRate);
    const int64_t releaseInSamples = samples_from_time(envelope.release.value, sampleRate);
    const int64_t time = current_sample - start_sample;

    float envelopeValue = 0.0f;

    if (ended) {
        // Calculate how long the note has been in release phase.
        int64_t releaseTime = current_sample - end_sample;

        // Determine the envelope level at the moment the note was released.
        int64_t releasePhaseStartTime = end_sample - start_sample;
        float releaseStartValue = 0.0f;
        if (releasePhaseStartTime < attackInSamples) {
            releaseStartValue = lerp_tension(0.0f, 1.0f, (float)releasePhaseStartTime / attackInSamples, envelope.attack.tension);
        }
        else if (releasePhaseStartTime < attackInSamples + decayInSamples) {
            releaseStartValue = lerp_tension(1.0f, envelope.sustain, (float)(releasePhaseStartTime - attackInSamples) / decayInSamples, envelope.decay.tension);
        }
        else {
            releaseStartValue = envelope.sustain;
        }

        // Now interpolate from the release start value to 0 over the release duration.
        if (releaseTime < releaseInSamples) {
            envelopeValue = lerp_tension(releaseStartValue, 0.0f, (float)releaseTime / releaseInSamples, envelope.release.tension);
        } else {
            envelopeValue = 0.0f;
            if (remove) {
                *remove = true; // Signal that the envelope is finished.
            }
        }
    }
    else {
        // Attack phase
        if (time < attackInSamples) {
            envelopeValue = lerp_tension(0.0f, 1.0f, (float)time / attackInSamples, envelope.attack.tension);
        }
        // Decay phase
        else if (time < attackInSamples + decayInSamples) {
            envelopeValue = lerp_tension(1.0f, envelope.sustain, (float)(time - attackInSamples) / decayInSamples, envelope.decay.tension);
        }
        // Sustain phase
        else {
            envelopeValue = envelope.sustain;
        }
    }

    return value * envelopeValue;
}

float adsr_from_cache(float value, int64_t current_sample, int64_t start_sample, int64_t end_sample,
    const AdsrEnvelope envelope, bool ended, bool *remove) {
    const int64_t time = current_sample - start_sample;

    float envelopeValue = 0.0f;

    if (ended) {
        const int64_t activeLen = envelope.cache.activeLength;
        const int64_t releaseInSamples = samples_from_time(envelope.release.value, SAMPLE_RATE);
        const int64_t releaseTime = current_sample - end_sample;

        float releaseStartValue = 0.0f;
        // Check if we fall into activeLen
        if (time < activeLen) {
            releaseStartValue = envelope.cache.active[time];
        }else {
            releaseStartValue = envelope.sustain;
        }

        if (releaseTime < releaseInSamples) {
            envelopeValue = lerp_tension(releaseStartValue, 0.0f, (float)releaseTime / releaseInSamples, envelope.release.tension);
        } else {
            envelopeValue = 0.0f;
            if (remove) {
                *remove = true; // Signal that the envelope is finished.
            }
        }
    }else {
        // Get the current envelope value from the cache if the time is before the sustain phase
        if (time < envelope.cache.activeLength) {
            envelopeValue = envelope.cache.active[current_sample - start_sample];
        } else {
            envelopeValue = envelope.sustain;
        }
    }

    return value * envelopeValue;
}

void adsr_cache_envelope(AdsrEnvelope *envelope, ARENA* arena, float sampleRate) {
    // First the active phase
    const int64_t attackInSamples  = samples_from_time(envelope->attack.value, sampleRate);
    const int64_t decayInSamples   = samples_from_time(envelope->decay.value, sampleRate);
    const int64_t releaseInSamples = samples_from_time(envelope->release.value, sampleRate);

    envelope->cache = (AdsrCache) {
        .active = (float*)arena_alloc(arena, (attackInSamples + decayInSamples) * sizeof(float)),
        .activeLength = attackInSamples + decayInSamples,
        .inactiveLength = releaseInSamples
    };

    for (int64_t i = 0; i < attackInSamples; i++) {
        envelope->cache.active[i] = lerp_tension(0.0f, 1.0f, (float)i / attackInSamples, envelope->attack.tension);
    }

    for (int64_t i = 0; i < decayInSamples; i++) {
        envelope->cache.active[i + attackInSamples] = lerp_tension(1.0f, envelope->sustain, (float)i / decayInSamples, envelope->decay.tension);
    }

    // That's it, the envelope is now cached.
}

AdsrEnvelope adsr_envelope_basic() {
    AdsrEnvelope envelope = {
        .attack = {
            .value = 0.01f,
            .tension = 0.5f
        },
        .decay ={
            .value = 0.5f,
            .tension = 0.8f
        },
        .sustain = 1.f,
        .release ={
            .value = 0.01f,
            .tension = -0.5f
        },
    };

    adsr_cache_envelope(&envelope, default_arena, SAMPLE_RATE);

    return envelope;
}
