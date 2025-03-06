#pragma once
#ifndef AUDIO_STATE_H
#define AUDIO_STATE_H

#include <raylib.h>
#include <tinycthread.h>

#define SAMPLE_RATE 44100
#define BUFFER_SIZE 1024
#define BIG_FIFO_BUFFER_SIZE SAMPLE_RATE
#define SMALL_FIFO_BUFFER_SIZE 1024
#define AUDIO_THREAD_PRIORITY 0 // 0 is considered default priority, if clicking occurs try setting to 1 which raises it to critical

typedef struct {
    float phase1;
    float phase2;
    mtx_t mutex;
    bool running;
    float* buffer;
    float* bigFifoBuffer;
    float* smallFifoBuffer;
    int bigFifoBufferIndex;
    int smallFifoBufferIndex;
    bool paused;
    bool reset;
} AudioState;

AudioState audio_state;
AudioStream stream;
thrd_t audio_thread;

#endif