#ifndef GENERATOR_H
#define GENERATOR_H

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
    float phase;
    int note;
} Voice;

typedef struct {
    Voice* voices;
    int voiceCount;
    int maxVoices;
    bool monophonic;
} VoiceStack;

typedef struct {
    /// Master frequency of the generator
    float frequency;

    /// Phase of the generator
    float phase;
    Waveform waveform;

    /// Voice stack (to be used for polyphony) (DO **NOT** ASSIGN TO THIS DIRECTLY)
    VoiceStack voices;

    /// Master volume/amplitude of the generator
    float amplitude; // [-1, 1] (inverting polarity is an option)

    /// Function to handle the generation of the waveform
    float (*generate)(void*, bool, bool); // Function pointer to the generate function

    /// Panning of the generator
    float panning; // [-1, 1] -1 is left, 1 is right
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

void generator_voice_process(int note, float frequency, float amplitude, bool remove, Generator* generator);
void generator_voice_remove(int note, Generator* generator);

float GenerateWaveform(void* generator, bool rightChannel, bool advancePhase);

#endif