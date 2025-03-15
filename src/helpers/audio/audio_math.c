#include "audio_math.h"

#include <stdio.h>
float wave_tables[NUM_WAVEFORMS][WAVE_TABLE_SIZE];
void init_math() {

#if defined(__x86_64__)
    if (has_avx2()){
        printf("Using AVX2\n");
    }else {
        printf("Warning: Not using AVX2\n");
    }
#endif

#if defined(__APPLE__) && defined(__arm64__)
    if (has_neon()){
        printf("Using NEON\n");
    }else {
        printf("Warning: Not using NEON\n");
    }
#endif

    // Sine
    for (int i = 0; i < WAVE_TABLE_SIZE; i++) {
        wave_tables[WT_SINE][i] = sinf(2.0f * PI * i / WAVE_TABLE_SIZE);
    }

    // Square
    for (int i = 0; i < WAVE_TABLE_SIZE; i++) {
        wave_tables[WT_SQUARE][i] = (i < WAVE_TABLE_SIZE / 2) ? 1.0f : -1.0f;
    }

    // Sawtooth
    for (int i = 0; i < WAVE_TABLE_SIZE; i++) {
        wave_tables[WT_SAWTOOTH][i] = 2.0f * i / WAVE_TABLE_SIZE - 1.0f;
    }

    // Triangle
    for (int i = 0; i < WAVE_TABLE_SIZE; i++) {
        wave_tables[WT_TRIANGLE][i] = (2.0f / PI) * asinf(sinf(2.0f * PI * i / WAVE_TABLE_SIZE));
    }

    // Simple noise
    for (int i = 0; i < WAVE_TABLE_SIZE; i++) {
        wave_tables[WT_NOISE][i] = 2.0f * ((float)rand() / RAND_MAX) - 1.0f;
    }
}

float quick_lookup(float phase, WaveformType waveform){
    // To prevent nasty aliasing, we need to lerp between the two closest values in the table
    float index = phase * (WAVE_TABLE_SIZE / (2.0f * PI));
    int i = (int)index;
    if (i >= WAVE_TABLE_SIZE) i = WAVE_TABLE_SIZE - 1;
    if (i < 0) i = 0;
    float t = index - i;
    return lerp(wave_tables[waveform][i], wave_tables[waveform][(i + 1) % WAVE_TABLE_SIZE], t);
}

float quick_calculate(__unused float phase, __unused WaveformType waveform) {
    // TODO: Implement this
    return 0;
}
