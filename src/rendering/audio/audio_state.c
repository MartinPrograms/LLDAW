#include "audio_state.h"

AudioState audio_state = {0};
AudioStream stream = {0};

thrd_t audio_playback_thread = nullptr;
thrd_t audio_process_thread = nullptr;

#include <string.h>  // For memcpy and memset

float * fifo_audio_to_normal(FifoAudioBuffer buffer, ARENA* arena) {
    int size = buffer.buffer_size;
    int head = buffer.head;
    int tail = buffer.tail;
    float* orig = buffer.buffer;

    float* new_buffer = (float*) arena_alloc(arena, size * sizeof(float));

    // Determine the number of valid samples.
    int count;
    if (head >= tail) {
        count = head - tail;
        memcpy(new_buffer, orig + tail, count * sizeof(float));
    } else {
        int first_segment = size - tail;
        memcpy(new_buffer, orig + tail, first_segment * sizeof(float));
        memcpy(new_buffer + first_segment, orig, head * sizeof(float));
        count = first_segment + head;
    }

    if (count < size) {
        memset(new_buffer + count, 0, (size - count) * sizeof(float));
    }

    return new_buffer;
}

void fifo_audio_write(FifoAudioBuffer* buffer, float* values, int count) {
    int size = buffer->buffer_size;
    int head = buffer->head;
    int tail = buffer->tail;
    float* orig = buffer->buffer;

    // Determine the number of valid samples.
    int valid;
    if (head >= tail) {
        valid = head - tail;
    } else {
        valid = size - tail + head;
    }

    // Determine the number of samples to write.
    int write = count;
    if (write > size - valid) {
        write = size - valid;
    }

    // Write the samples.
    if (tail + write < size) {
        memcpy(orig + tail, values, write * sizeof(float));
    } else {
        int first_segment = size - tail;
        memcpy(orig + tail, values, first_segment * sizeof(float));
        memcpy(orig, values + first_segment, (write - first_segment) * sizeof(float));
    }

    // Update the tail.
    buffer->tail = (tail + write) % size;
}