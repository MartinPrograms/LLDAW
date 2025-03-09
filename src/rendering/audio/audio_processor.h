#ifndef AUDIO_PROCESSOR_H
#define AUDIO_PROCESSOR_H

#include <raylib.h>
#include <tinycthread.h>
#include <math.h>
#include <stdio.h>
#include "generator.h"
#include "../../helpers/include.h"
#include "../../helpers/basic/definitions.h" // not automatically included by include.h.
#include "../../sequencing/sequencer.h"

void InitAudio();
int AudioPlaybackThread(void* audio_state);
int AudioProcessingThread(void* audio_state);
void CalculateBuffer(float *buffer, int bufferSize);
void play(__unused void* userdata);
void pauseCallback(__unused void* userdata);
void stop(__unused void* userdata);

#endif // AUDIO_PROCESSOR_H