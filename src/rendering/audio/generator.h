#ifndef GENERATOR_H
#define GENERATOR_H

#define UNISON_MAX_CENTS 100 // Maximum detune in cents

#include <stdint.h>
#include "adsr.h"
#include "../../helpers/basic/arena.h"

// This is a simple audio generator that generates a sine wave at a given frequency. Think of it as a very very *very* simple synthesizer. No ADSR, no filters, no effects, just a wave of your choice, with a frequency of your choice.

typedef enum{
    SINE,
    SQUARE,
    SAWTOOTH,
    TRIANGLE,
    NOISE
} Waveform;

/// Struct to handle voice generation (a voice is a single sound source)
typedef struct {
    float frequency;
    float amplitude;
    float panning;
    float* phase;
    int note;
    int64_t startSample;
    int64_t endSample;
    bool active; // The moment active is false, the release phase starts
    bool remove; // The moment remove is true, the voice is removed from the stack
    ARENA* arena;
} Voice;

typedef struct {
    Voice* voices;
    int head;
    int tail;
    int count;
    int maxVoices;
    bool monophonic;
} VoiceStack;

typedef struct {
    /// Master frequency of the generator
    float frequency;
    /// Phase of the generator
    float phase;
    /// Waveform of the generator
    Waveform waveform;

    /// Unison of the generator [1+]
    int unison;
    /// Unison detune of the generator [0, 1]
    float unison_detune;
    /// Phase randomization of the generator [0, 1]
    float phase_randomization;

    /// Envelope of the generator
    AdsrEnvelope envelope;
    /// Master volume/amplitude of the generator
    float amplitude; // [-1, 1] (inverting polarity is an option)
    /// Master panning of the generator
    float panning; // [-1, 1] -1 is left, 1 is right

    /// Voice stack (to be used for polyphony) (DO **NOT** ASSIGN TO THIS DIRECTLY)
    VoiceStack active_voices;
    /// Inactive voices (for release phase)
    VoiceStack inactive_voices;

    /// Function to handle the generation of the waveform
    float (*generate)(void*, bool, bool); // Function pointer to the generate function
} Generator;

typedef struct {
    Generator* generators;
    int generatorCount;
    int generatorCapacity;
    ARENA* arena;
} GeneratorState;

GeneratorState generator_init(int capacity);
void generator_add(GeneratorState* state, Generator generator);
void generator_remove(GeneratorState* state, int index);
void generator_free(GeneratorState* state);

void generator_voice_process(int note, float frequency, float amplitude, float pan, bool deactivate, Generator* generator);
void generator_voice_remove(int note, Generator* generator);
void generator_voice_deactivate(int note, Generator* generator);
void generator_voice_cleanse(Generator* generator);
void generator_kill_all_voices(Generator* generator);
void generator_kill_voice(int note, Generator* generator);

float GenerateWaveform(void* generator, bool rightChannel, bool advancePhase);

#endif