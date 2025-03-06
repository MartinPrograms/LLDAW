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

typedef struct {
    float frequency;
    float phase;
    Waveform waveform;
    float amplitude; // [-1, 1] (inverting polarity is an option)
    float (*generate)(float frequency, float phase, float amplitude, Waveform waveform);
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

float GenerateWaveform(float frequency, float phase, float amplitude, Waveform waveform);

#endif