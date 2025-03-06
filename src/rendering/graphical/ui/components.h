#ifndef COMPONENTS_H
#define COMPONENTS_H

#include <clay.h>

extern int CustomComponentIndex;
void WaveformVisualizer(const float* buffer, int bufferSize);
void SpectrumVisualizer(const float* buffer, int bufferSize);

#endif