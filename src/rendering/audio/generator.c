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

        generator.active_voices = (VoiceStack) {
                .voices = (Voice *)arena_alloc(default_arena, size * sizeof(Voice)),
                .count = 0,
                .head = 0,
                .tail = 0,
                .maxVoices = size,
                .monophonic = false
        };

        generator.inactive_voices = (VoiceStack) {
                .voices = (Voice *)arena_alloc(default_arena, size * sizeof(Voice)),
                .count = 0,
                .head = 0,
                .tail = 0,
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

void generator_remove_oldest_active(Generator *generator) {
    if (generator->active_voices.count > 0) {
        // Find the oldest voice
        int index = generator->active_voices.head;
        generator->active_voices.voices[index].remove = true; // Mark the voice as removed.

        int inactive_index = generator->inactive_voices.tail;
        generator->inactive_voices.voices[inactive_index] = generator->active_voices.voices[index];

        generator->inactive_voices.tail = (generator->inactive_voices.tail + 1) % generator->inactive_voices.maxVoices;
        generator->inactive_voices.count++;

        generator->active_voices.head = (generator->active_voices.head + 1) % generator->active_voices.maxVoices;
        generator->active_voices.count--;
    }
}

void generator_remove_oldest_inactive(Generator *generator) {
    if (generator->inactive_voices.count > 0) {
        int index = generator->inactive_voices.head;
        generator->inactive_voices.head = (generator->inactive_voices.head + 1) % generator->inactive_voices.maxVoices;
        generator->inactive_voices.count--;
        arena_destroy(generator->inactive_voices.voices[index].arena);
   }
}

void generator_voice_remove(int note, Generator *generator) {
    // Simply mark the voice for removal, it will be removed in the next cleanse.
    int count = generator->active_voices.count;
    int index = generator->active_voices.head;
    for (int i = 0; i < count; i++) {
        if (generator->active_voices.voices[index].note == note) {
            generator->active_voices.voices[index].remove = true;
        }
        index = (index + 1) % generator->active_voices.maxVoices;
    }
}

void generator_voice_deactivate(int note, Generator *generator) {
    int count = generator->active_voices.count;
    int max = generator->active_voices.maxVoices;
    int head = generator->active_voices.head;
    int newCount = 0;

    Voice temp[max];

    for (int i = 0; i < count; i++) {
        int idx = (head + i) % max;
        if (generator->active_voices.voices[idx].note == note) {
            generator->active_voices.voices[idx].active = false;
            generator->active_voices.voices[idx].endSample = audio_state.sample_number;

            if (generator->inactive_voices.count >= generator->inactive_voices.maxVoices) {
                generator_remove_oldest_inactive(generator);
            }

            int inactive_index = generator->inactive_voices.tail;
            generator->inactive_voices.voices[inactive_index] = generator->active_voices.voices[idx];
            generator->inactive_voices.tail = (generator->inactive_voices.tail + 1) % generator->inactive_voices.maxVoices;
            generator->inactive_voices.count++;
        }else {
            temp[newCount] = generator->active_voices.voices[idx];
            newCount++;
        }
    }

    for (int i = 0; i < newCount; i++) {
        generator->active_voices.voices[i] = temp[i];
    }

    generator->active_voices.count = newCount;
    generator->active_voices.head = 0;
    generator->active_voices.tail = newCount % max;
}

void generator_voice_cleanse(Generator *generator) {
    int activeCount = generator->active_voices.count;
    int activeMax = generator->active_voices.maxVoices;

    Voice tempActive[activeMax];
    int newActiveCount = 0;

    for (int i = 0; i < activeCount; i++) {
        int idx = (generator->active_voices.head + i) % activeMax;
        if (generator->active_voices.voices[idx].remove == true) {
            arena_destroy(generator->active_voices.voices[idx].arena);
        }
        else {
            tempActive[newActiveCount] = generator->active_voices.voices[idx];
            newActiveCount++;
        }
    }

    for (int i = 0; i < newActiveCount; i++) {
        generator->active_voices.voices[i] = tempActive[i];
    }
    generator->active_voices.count = newActiveCount;
    generator->active_voices.head = 0;
    generator->active_voices.tail = newActiveCount % activeMax;

    int inactiveCount = generator->inactive_voices.count;
    int inactiveMax = generator->inactive_voices.maxVoices;
    Voice tempInactive[inactiveMax];
    int newInactiveCount = 0;

    for (int i = 0; i < inactiveCount; i++) {
        int idx = (generator->inactive_voices.head + i) % inactiveMax;
        if (generator->inactive_voices.voices[idx].remove == true) {
            arena_destroy(generator->inactive_voices.voices[idx].arena);
        }
        else {
            tempInactive[newInactiveCount] = generator->inactive_voices.voices[idx];
            newInactiveCount++;
        }
    }

    for (int i = 0; i < newInactiveCount; i++) {
        generator->inactive_voices.voices[i] = tempInactive[i];
    }

    generator->inactive_voices.count = newInactiveCount;
    generator->inactive_voices.head = 0;
    generator->inactive_voices.tail = newInactiveCount % inactiveMax;
}

void generator_voice_process(int note, float frequency, float amplitude, float pan, bool deactivate, Generator *generator) {
    if (deactivate) {
        generator_voice_deactivate(note, generator);
        return;
    }

    // Make sure we don't exceed our maximum voice count.
    if (generator->active_voices.count >= generator->active_voices.maxVoices) {
        generator_remove_oldest_active(generator);
    }

    // Add the new voice.
    int index = generator->active_voices.tail;
    ARENA* new_arena = arena_create(sizeof(float) * generator->unison);
    generator->active_voices.voices[index] = (Voice) {
        .frequency = frequency,
        .amplitude = amplitude,
        .panning = pan,  // TODO: Adjust panning as needed.
        .arena = new_arena,
        .phase = (float*)arena_alloc(new_arena, sizeof(float) * generator->unison),
        .note = note,
        .active = true,
        .startSample = audio_state.sample_number,
    };

    // Randomize phase for each unison voice.
    for (int i = 0; i < generator->unison; i++) {
        float random = generator->phase_randomization; // % to differ from base, if its 1 we must add full randomization
        generator->active_voices.voices[index].phase[i] = random_value(0, 2.0f * PI) * random;
    }

    generator->active_voices.count++;
    generator->active_voices.tail = (generator->active_voices.tail + 1) % generator->active_voices.maxVoices;
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
    amplitude = adsr_from_cache(amplitude, audio_state.sample_number, voice->startSample, voice->endSample, generator->envelope, !voice->active, &remove);

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

    // Loop through all active & inactive voices
    for (int i = 0; i < generator->active_voices.count; i++) {
        int index = (generator->active_voices.head + i) % generator->active_voices.maxVoices;
        generate_voice(rightChannel, advancePhase, generator, index, generator->active_voices.voices, &value);
    }

    for (int i = 0; i < generator->inactive_voices.count; i++) {
        int index = (generator->inactive_voices.head + i) % generator->inactive_voices.maxVoices;
        generate_voice(rightChannel, advancePhase, generator, index, generator->inactive_voices.voices, &value);
    }

    // Apply master amplitude
    value = gain(value, generator->amplitude);
    // Apply master panning
    value = panning(value, generator->panning, rightChannel);

    generator_voice_cleanse(generator);

    return value;
}

void generator_kill_all_voices(Generator* generator){
    for (int i = 0; i < generator->active_voices.count; i++) {
        generator->active_voices.voices[i].remove = true;
    }

    generator_voice_cleanse(generator);
}

void generator_kill_voice(int note, Generator *generator) {
    for (int i = 0; i < generator->active_voices.count; i++) {
        if (generator->active_voices.voices[i].note == note) {
            generator->active_voices.voices[i].remove = true;
        }
    }

    generator_voice_cleanse(generator);
}
