#include <math.h>
#include <raylib.h>
#include <stdlib.h>
#include "generator.h"

GeneratorState generator_init(int capacity) {
    GeneratorState state = {
            .generators = NULL,
            .generatorCount = 0,
            .generatorCapacity = capacity,
            .arena = arena_create(capacity * sizeof(Generator)) // allocate a megabyte of memory for the arena
    };

    state.generators = (Generator *)arena_alloc(state.arena, capacity * sizeof(Generator));

    return state;
}

void generator_add(GeneratorState *state, Generator generator) {
    if (state->generatorCount < state->generatorCapacity) {
        state->generators[state->generatorCount++] = generator;
    }
}

void generator_remove(GeneratorState *state, int index) {
    if (index < state->generatorCount) {
        for (int i = index; i < state->generatorCount - 1; i++) {
            state->generators[i] = state->generators[i + 1];
        }
        state->generatorCount--;
    }
}

void generator_free(GeneratorState *state) {
    arena_destroy(state->arena);
}

float GenerateWaveform(float frequency, float phase, float amplitude, Waveform waveform) {
    float value = 0;
    switch (waveform) {
        case SINE:
            value = sinf(2 * PI * frequency * phase) * amplitude;
            break;
        case SQUARE:
            value = sinf(2 * PI * frequency * phase) > 0 ? amplitude : -amplitude;
            break;
        case SAWTOOTH:
            value = fmodf(phase, 1) * amplitude;
            break;
        case TRIANGLE:
            value = 1 - 4 * fabsf(roundf(phase) - phase) * amplitude;
            break;
        case NOISE:
            value = (rand() % 1000) / 1000.0f * amplitude;
            break;
    }
    return value;
}

