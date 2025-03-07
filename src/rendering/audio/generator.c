#include <math.h>
#include <raylib.h>
#include <stdlib.h>
#include "generator.h"
#include "audio_state.h"
#include "../../src/helpers/audio/audio_math.h"

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

float GenerateWaveform(void* generator_void, bool  rightChannel, bool advancePhase) {
    Generator* generator = (Generator*)generator_void;
    float value = 0;

    // Capture parameters at start of calculation
    float phase = generator->phase;
    const float frequency = generator->frequency;
    const float amplitude = generator->amplitude;
    const Waveform waveform = generator->waveform;

    // Phase increment FIRST (before wrapping)
    if (advancePhase) {
        generator->phase += 2.0f * PI * frequency / SAMPLE_RATE;

        // Phase wrapping with modulus (keeps phase in [0, 2π) range)
        generator->phase = fmodf(generator->phase, 2.0f * PI);
    }
    // Handle negative phase values (fmod() can return negatives)
    if (generator->phase < 0)
        generator->phase += 2.0f * PI;

    switch (waveform) {
        case SINE:
            value = sinf(phase) * amplitude;  // Use original phase value
            break;
        case SAWTOOTH:
            value = (2.0f * phase / (2.0f * PI) - 1.0f) * amplitude;
            break;
        case SQUARE:
            value = (phase < PI) ? amplitude : -amplitude;
            break;
        case TRIANGLE:
            value = (2.0f / PI) * asinf(sinf(phase)) * amplitude;
            break;
        case NOISE:
            value = (float)GetRandomValue(-1000, 1000) / 1000.0f * amplitude;
            break;
        default:
            break;
    }

    // Apply panning (0 is center, -1 is left, 1 is right)
    const float pan = generator->panning;  // pan range: -1.0f (left) to 1.0f (right)
    value = panning(value, pan, rightChannel);

    return value;
}