#include "audio_state.h"

AudioState audio_state = {0};
AudioStream stream = {0};

thrd_t audio_playback_thread = nullptr;
thrd_t audio_process_thread = nullptr;

#include <string.h>  // For memcpy and memset

// Write new samples into the FIFO, appending them at the head.
void fifo_audio_write(FifoAudioBuffer* buffer, float* values, int count) {
    for (int i = 0; i < count; ++i) {
        buffer->buffer[buffer->head] = values[i];
        buffer->head = (buffer->head + 1) % buffer->buffer_size;

        if (buffer->buffer_index < buffer->buffer_size) {
            buffer->buffer_index++;
        } else {
            // Overwrite the oldest data by moving the tail forward
            buffer->tail = (buffer->tail + 1) % buffer->buffer_size;
        }
    }
}

// Convert the FIFO buffer to a linear (normal) buffer.
// The valid data is assumed to be in the range from tail to head.
float* fifo_audio_to_normal(FifoAudioBuffer buffer, ARENA* arena) {
    // Allocate memory from the arena for the linear buffer
    float* linear_buffer = (float*)arena_alloc(arena, buffer.buffer_index * sizeof(float));
    if (buffer.buffer_index == 0) {
        return linear_buffer; // Return empty buffer if no data
    }

    int current = buffer.tail;
    for (int i = 0; i < buffer.buffer_index; ++i) {
        linear_buffer[i] = buffer.buffer[current];
        current = (current + 1) % buffer.buffer_size;
    }

    return linear_buffer;
}