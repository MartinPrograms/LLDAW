#pragma once
#ifndef AUDIO_STATE_H
#define AUDIO_STATE_H

#include <raylib.h>
#include <stdint.h>
#include <tinycthread.h>
#include "generator.h"

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
    int buffer_tail;
    int buffer_head;

    /// Stereo audio buffer, this one goes to the visualizer, this is FIFO.
    FifoAudioBuffer fifo_buffer;

    /// Stereo audio buffer, this one goes to the visualizer, this is FIFO.
    FifoAudioBuffer big_fifo_buffer;
} AudioBuffers;

typedef struct {
    mtx_t state_mutex;
    cnd_t resume_playback_cnd; // This is called when resume_processing_cnd finishes fully.
    cnd_t resume_processing_cnd;
    bool started;

    mtx_t processing_mutex;

    AudioBuffers audio_buffers;
    double time_to_render_buffer;

    GeneratorState generator_state;

    bool running; // Stops the entire thread, once turned off it can't be turned back on.
    bool paused; // Pauses audio processing
    bool reset; // Resets audio buffers to 0

    float master_pan;
    float master_volume;

    // Use an unsigned 64 bit integer to store the current sample number
    int64_t sample_number;
    float current_beat;
} AudioState;

extern AudioState audio_state;
extern AudioStream stream;
extern thrd_t audio_playback_thread;
extern thrd_t audio_process_thread;

float* fifo_audio_to_normal(FifoAudioBuffer buffer, ARENA* arena);
void fifo_audio_write(FifoAudioBuffer* buffer, float* values, int count);

#endif