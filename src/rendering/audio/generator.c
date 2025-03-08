#include <math.h>
#include <raylib.h>
#include <stdlib.h>
#include "generator.h"

#include <stdio.h>

#include "audio_state.h"
#include "../../helpers/include.h"
#include "../../src/helpers/audio/audio_math.h"

GeneratorState generator_init(int capacity) {
    GeneratorState state = {
            .generators = nullptr,
            .generatorCount = 0,
            .generatorCapacity = capacity,
            .arena = arena_create(capacity * sizeof(Generator)) // allocate a megabyte of memory for the arena
    };

    state.generators = (Generator *)arena_alloc(state.arena, capacity * sizeof(Generator));

    return state;
}

void generator_add(GeneratorState *state, Generator generator) {
    if (state->generatorCount < state->generatorCapacity) {
        generator.voices = (VoiceStack) {
                .voices = (Voice *)arena_alloc(default_arena, 64 * sizeof(Voice)),
                .voiceCount = 0,
                .maxVoices = 64,
                .monophonic = false
        };
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

void generator_remove_oldest(Generator *generator) {
    if (generator->voices.voiceCount > 0) {
        // Remove the oldest voice (assumed at index 0)
        for (int i = 1; i < generator->voices.voiceCount; i++) {
            generator->voices.voices[i - 1] = generator->voices.voices[i];
        }
        generator->voices.voiceCount--;
    }
}

void generator_voice_remove(int note, Generator *generator) {
    // Remove ALL voices that match the given note.
    for (int i = 0; i < generator->voices.voiceCount;) {
        if (generator->voices.voices[i].note == note) {
            // Shift remaining voices left by one.
            for (int j = i + 1; j < generator->voices.voiceCount; j++) {
                generator->voices.voices[j - 1] = generator->voices.voices[j];
            }
            generator->voices.voiceCount--;
            // Do not increment i, because a new voice now occupies index i.
        } else {
            i++;
        }
    }
}

void generator_voice_process(int note, float frequency, float amplitude, bool remove, Generator *generator) {
    if (remove) {
        // On note off, remove any voice with the matching note and do nothing else.
        printf("Removing voice %d\n", note);
        generator_voice_remove(note, generator);
        return;
    }

    // (Optional) If you want to retrigger the note rather than stacking voices,
    // remove any voice already playing for this note.
    generator_voice_remove(note, generator);

    // Make sure we don't exceed our maximum voice count.
    if (generator->voices.voiceCount >= generator->voices.maxVoices) {
        generator_remove_oldest(generator);
    }

    // Add the new voice.
    generator->voices.voices[generator->voices.voiceCount++] = (Voice) {
        .frequency = frequency,
        .amplitude = amplitude,
        .panning = 0,  // TODO: Adjust panning as needed.
        .phase = 0,
        .note = note
    };
}


void generate_voice(bool rightChannel, bool advancePhase, Generator *generator, int index, float *value) {
    Voice* voice = &generator->voices.voices[index];
    float phase = voice->phase;
    const float frequency = voice->frequency;
    const float amplitude = voice->amplitude;
    const Waveform waveform = generator->waveform;
    float output = 0.0f;

    // Phase increment FIRST (before wrapping)
    if (advancePhase) {
        voice->phase += 2.0f * PI * frequency / SAMPLE_RATE;

        // Phase wrapping with modulus (keeps phase in [0, 2π) range)
        voice->phase = fmodf(voice->phase, 2.0f * PI);
    }
    // Handle negative phase values (fmod() can return negatives)
    if (voice->phase < 0)
        voice->phase += 2.0f * PI;

    switch (waveform) {
        case SINE:
            output = sinf(phase) * amplitude;  // Use original phase value
            break;
        case SAWTOOTH:
            output = (2.0f * phase / (2.0f * PI) - 1.0f) * amplitude;
            break;
        case SQUARE:
            output = (phase < PI) ? amplitude : -amplitude;
            break;
        case TRIANGLE:
            output = (2.0f / PI) * asinf(sinf(phase)) * amplitude;
            break;
        case NOISE:
            output = (float)GetRandomValue(-1000, 1000) / 1000.0f * amplitude;
            break;
        default:
            break;
    }

    // Apply panning (0 is center, -1 is left, 1 is right)
    const float pan = generator->panning;  // pan range: -1.0f (left) to 1.0f (right)
    output = panning(output, pan, rightChannel);

    *value += output;
}

float GenerateWaveform(void* generator_void, bool  rightChannel, bool advancePhase) {
    Generator* generator = (Generator*)generator_void;
    float value = 0;

    // Capture parameters at start of calculation
    for (int i = 0; i < generator->voices.voiceCount; i++) {
        generate_voice(rightChannel, advancePhase, generator, i, &value);
    }

    return value;
}
