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
    float leftGain, rightGain;
    if (rightChannel) {
        // Calculate gains for right channel
        rightGain = (pan + 1.0f) * 0.5f;  // 0.0f to 1.0f
        result *= rightGain;
    } else {
        // Calculate gains for left channel
        leftGain = (1.0f - pan) * 0.5f;   // 0.0f to 1.0f
        result *= leftGain;
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

#endif