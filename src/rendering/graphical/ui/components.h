#ifndef COMPONENTS_H
#define COMPONENTS_H

#include <clay.h>

typedef struct {
    const char* text;
    void* userData;
    void (*callback)(void*);
} ButtonData;

extern int CustomComponentIndex;
void WaveformVisualizer(const float* buffer, int bufferSize);
void SpectrumVisualizer(const float* buffer, int bufferSize);

#endif