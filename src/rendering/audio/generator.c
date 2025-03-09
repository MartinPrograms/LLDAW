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
        int size = 16; // 16 voices per generator
        generator.voices = (VoiceStack) {
                .voices = (Voice *)arena_alloc(default_arena, size * sizeof(Voice)),
                .deactivatedVoices = (Voice *)arena_alloc(default_arena, size * sizeof(Voice)),
                .voiceCount = 0,
                .deactivatedVoiceCount = 0,
                .maxVoices = size,
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
        // Find the oldest voice
        int oldestIndex = 0;
        int64_t oldestSample = generator->voices.voices[0].startSample;
        for (int i = 1; i < generator->voices.voiceCount; i++) {
            if (generator->voices.voices[i].startSample < oldestSample) {
                oldestSample = generator->voices.voices[i].startSample;
                oldestIndex = i;
            }
        }

        // Move it to the deactivated voices, and shift the rest left by one.
        generator->voices.voices[oldestIndex].remove = true;
        generator->voices.deactivatedVoices[generator->voices.deactivatedVoiceCount++] = generator->voices.voices[oldestIndex];

        for (int i = oldestIndex + 1; i < generator->voices.voiceCount; i++) {
            generator->voices.voices[i - 1] = generator->voices.voices[i];
        }
        generator->voices.voiceCount--;
    }
}

void generator_voice_remove(int note, Generator *generator) {
    // Simply mark the voice for removal, it will be removed in the next cleanse.
    for (int i = 0; i < generator->voices.voiceCount; i++) {
        if (generator->voices.voices[i].note == note) {
            generator->voices.voices[i].remove = true;
        }
    }
}

void generator_voice_deactivate(int note, Generator *generator) {
    for (int i = 0; i < generator->voices.voiceCount; i++) {
        if (generator->voices.voices[i].note == note) {
            generator->voices.voices[i].active = false;
            generator->voices.voices[i].endSample = audio_state.sample_number;

            // transfer the voice to the deactivated voices
            generator->voices.deactivatedVoices[generator->voices.deactivatedVoiceCount++] = generator->voices.voices[i];
            // Shift remaining voices left by one.
            for (int j = i + 1; j < generator->voices.voiceCount; j++) {
                generator->voices.voices[j - 1] = generator->voices.voices[j];
            }

            generator->voices.voiceCount--;
        }
    }
}

void generator_voice_cleanse(Generator *generator) {
    // Remove all voices that have been marked for removal. In both active and deactivated voices.
    for (int i = 0; i < generator->voices.voiceCount; i++) {
        if (generator->voices.voices[i].remove) {
            for (int j = i + 1; j < generator->voices.voiceCount; j++) {
                generator->voices.voices[j - 1] = generator->voices.voices[j];
            }
            generator->voices.voiceCount--;
            i--;
        }
    }

    for (int i = 0; i < generator->voices.deactivatedVoiceCount; i++) {
        if (generator->voices.deactivatedVoices[i].remove) {
            for (int j = i + 1; j < generator->voices.deactivatedVoiceCount; j++) {
                generator->voices.deactivatedVoices[j - 1] = generator->voices.deactivatedVoices[j];
            }
            generator->voices.deactivatedVoiceCount--;
            i--;
        }
    }
}

void generator_voice_process(int note, float frequency, float amplitude, bool deactivate, Generator *generator) {
    if (deactivate) {
        generator_voice_deactivate(note, generator);
        return;
    }

    // Make sure we don't exceed our maximum voice count.
    if (generator->voices.voiceCount >= generator->voices.maxVoices) {
        generator_remove_oldest(generator);
    }

    // Add the new voice.
    generator->voices.voices[generator->voices.voiceCount++] = (Voice) {
        .frequency = frequency,
        .amplitude = amplitude,
        .panning = 0,  // TODO: Adjust panning as needed.
        .phase = (float*)arena_alloc(default_arena, sizeof(float) * generator->unison),
        .note = note,
        .active = true,
        .startSample = audio_state.sample_number,
    };

    // Randomize phase for each unison voice.
    for (int i = 0; i < generator->unison; i++) {
        float random = generator->phase_randomization; // % to differ from base, if its 1 we must add full randomization
        generator->voices.voices[generator->voices.voiceCount - 1].phase[i] = random_value(0, 2.0f * PI) * random;
    }
}


void generate_waveform(Generator *generator, float phase, float amplitude, float *output) {
    const Waveform waveform = generator->waveform;
    float out = 0.0;

    switch (waveform) {
        case SINE:
            out = quick_lookup(phase, WT_SINE) * amplitude;  // Use original phase value
            break;
        case SAWTOOTH:
            out = quick_lookup(phase, WT_SAWTOOTH) * amplitude;
            break;
        case SQUARE:
            out = quick_lookup(phase, WT_SQUARE) * amplitude;
            break;
        case TRIANGLE:
            out = quick_lookup(phase, WT_TRIANGLE) * amplitude;
            break;
        case NOISE:
            out = quick_lookup(phase, WT_NOISE) * amplitude;
            break;
        default:
            break;
    }

    *output += out;
}

void generate_voice(bool rightChannel, bool advancePhase, Generator *generator, int index, Voice* voices, float *value) {
    Voice* voice = &voices[index];
    float mainPhase = voice->phase[0];
    const float frequency = voice->frequency;

    float amplitude = voice->amplitude;
    bool remove = false;
    amplitude = adsr_envelope_apply(amplitude, audio_state.sample_number, voice->startSample, voice->endSample, generator->envelope, !voice->active, SAMPLE_RATE, &remove);

    if (remove) {
        voice->remove = true;
        return;
    }

    float output = 0.0f;

    int unisonCount = generator->unison;
    float unisonDetune = generator->unison_detune; // % of the UNISON_MAX_CENTS

    if (unisonCount > 1) {
        const float max_cents = UNISON_MAX_CENTS * unisonDetune;
        float unisonSum = 0.0f;
        const float base_pan = voice->panning;

        // Loop through each unison voice.
        for (int i = 0; i < unisonCount; i++) {
            // Calculate a detune offset in cents. This spreads voices evenly from -max_cents to +max_cents.
            float detuneCents = 0.0f;
            // Map i from [0, unisonCount-1] to [-1, +1] then scale by max_cents.
            detuneCents = max_cents * ((2.0f * i / (unisonCount - 1)) - 1.0f);

            // Convert detune (in cents) to a frequency multiplier.
            float detuneFactor = powf(2.0f, detuneCents / 1200.0f);
            float detunedFrequency = voice->frequency * detuneFactor;
            float phaseIncrement = 2.0f * PI * detunedFrequency / SAMPLE_RATE;

            // Use the phase for this unison voice (assuming voice->phase is an array sized to at least unisonCount)
            float currentPhase = voice->phase[i];
            float voiceOutput = 0.0f;

            // Generate the waveform sample for this sub-voice.
            generate_waveform(generator, currentPhase, amplitude, &voiceOutput);
            // Apply panning for each sub-voice individually.
            float panningValue = base_pan + (detuneCents / max_cents) * (1.0f - base_pan);
            voiceOutput = panning(voiceOutput, panningValue, rightChannel);

            unisonSum += voiceOutput;

            // Advance the phase if needed.
            if (advancePhase) {
                currentPhase += phaseIncrement;
                if (currentPhase > 2.0f * PI)
                    currentPhase -= 2.0f * PI;
            }
            voice->phase[i] = currentPhase;
        }
        // Normalize the summed output.
        output = unisonSum / (float)unisonCount;
    } else {
        generate_waveform(generator, mainPhase, amplitude, &output);
        // Apply main voice panning for non-unison case
        output = panning(output, voice->panning, rightChannel);
        if (advancePhase) {
            voice->phase[0] += 2.0f * PI * frequency / SAMPLE_RATE;
            if (voice->phase[0] > 2.0f * PI) {
                voice->phase[0] -= 2.0f * PI;
            }
        }
    }

    *value += output;
}

float GenerateWaveform(void* generator_void, bool  rightChannel, bool advancePhase) {
    Generator* generator = (Generator*)generator_void;
    float value = 0;

    // Capture parameters at start of calculation
    for (int i = 0; i < generator->voices.voiceCount; i++) {
        generate_voice(rightChannel, advancePhase, generator, i, generator->voices.voices, &value);
    }

    for (int i = 0; i < generator->voices.deactivatedVoiceCount; i++) {
        generate_voice(rightChannel, advancePhase, generator, i, generator->voices.deactivatedVoices, &value);
    }

    // Apply master amplitude
    value = gain(value, generator->amplitude);
    // Apply master panning
    value = panning(value, generator->panning, rightChannel);

    generator_voice_cleanse(generator);

    return value;
}
