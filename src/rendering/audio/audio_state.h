#pragma once
#ifndef AUDIO_STATE_H
#define AUDIO_STATE_H

#include <raylib.h>
#include <tinycthread.h>
#include "generator.h"

#define SAMPLE_RATE 44100
#define BUFFER_SIZE 1024
#define BIG_FIFO_BUFFER_SIZE SAMPLE_RATE
#define SMALL_FIFO_BUFFER_SIZE 1024
#define AUDIO_THREAD_PRIORITY 0 // 0 is considered default priority, if clicking occurs try setting to 1 which raises it to critical
#define AUDIO_BUFFER_COUNT 2 // Increasing this adds slightly more latency, but can help with audio glitches

typedef struct {
    float* buffer;
    int buffer_size;
} AudioBuffer; // This is a simple struct to hold the buffer and its size

typedef struct {
    float* buffer;
    int buffer_size;
    int buffer_index;
    int tail;
    int head;
} FifoAudioBuffer;

typedef struct {
    /// Stereo audio buffer, this one goes to the actual headphones
    AudioBuffer main_buffer;

    /// List of buffers to be rendered, before sent to the main buffer. Usually 2, or 3. Stereo audio.
    AudioBuffer* buffers;
    int buffer_count;
    int buffer_index;

    /// Stereo audio buffer, this one goes to the visualizer, this is FIFO.
    FifoAudioBuffer fifo_buffer;

    /// Stereo audio buffer, this one goes to the visualizer, this is FIFO.
    FifoAudioBuffer big_fifo_buffer;
} AudioBuffers;

typedef struct {
    mtx_t playback_mutex;
    mtx_t processing_mutex;

    AudioBuffers audio_buffers;
    GeneratorState generator_state;

    bool running; // Stops the entire thread, once turned off it can't be turned back on.
    bool paused; // Pauses audio processing
    bool reset; // Resets audio buffers to 0

    float master_pan;
    float master_volume;
} AudioState;

extern AudioState audio_state;
extern AudioStream stream;
extern thrd_t audio_playback_thread;
extern thrd_t audio_process_thread;

float* fifo_audio_to_normal(FifoAudioBuffer buffer, ARENA* arena);
void fifo_audio_write(FifoAudioBuffer* buffer, float* values, int count);

#endif