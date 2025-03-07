#include "audio_processor.h"

#include "../../helpers/audio/audio_math.h"

void CalculateBuffer(float *buffer, int bufferSize) {
    for (int i = 0; i < bufferSize; i++) {
        float sumL = 0;
        float sumR = 0;

        for (int j = 0; j < audio_state.generator_state.generatorCount; j++) {
            Generator *generator = &audio_state.generator_state.generators[j];
            sumL += generator->generate(generator, true, false);
            sumR += generator->generate(generator, false, true); // we advance the phase here!
        }

        sumL = sumL * audio_state.master_volume; // doing this here, to prevent it from going over 1.0
        sumR = sumR * audio_state.master_volume;

        sumL = panning(sumL, audio_state.master_pan, false);
        sumR = panning(sumR, audio_state.master_pan, true);

        sumL = fmin(fmax(sumL, -1.0f), 1.0f);
        sumR = fmin(fmax(sumR, -1.0f), 1.0f);

        buffer[i * 2] = sumL;
        buffer[i * 2 + 1] = sumR;
    }
}

void create_buffers(AudioState *state) {
    state->audio_buffers = (AudioBuffers){0};

    state->audio_buffers.main_buffer.buffer = (float*)malloc(BUFFER_SIZE * 2 * sizeof(float)); // times 2 because we have 2 channels
    state->audio_buffers.main_buffer.buffer_size = BUFFER_SIZE * 2;

    state->audio_buffers.buffers = (AudioBuffer*)malloc(AUDIO_BUFFER_COUNT * sizeof(AudioBuffer));
    for (int i = 0; i < AUDIO_BUFFER_COUNT; i++) {
        state->audio_buffers.buffers[i].buffer = (float*)malloc(BUFFER_SIZE * 2 * sizeof(float)); // times 2 because we have 2 channels
        state->audio_buffers.buffers[i].buffer_size = BUFFER_SIZE * 2;
    }

    state->audio_buffers.fifo_buffer.buffer = (float*)malloc(BUFFER_SIZE * 2 * sizeof(float)); // times 2 because we have 2 channels
    state->audio_buffers.fifo_buffer.buffer_size = BUFFER_SIZE * 2;
    state->audio_buffers.fifo_buffer.buffer_index = 0;
    state->audio_buffers.fifo_buffer.tail = 0;
    state->audio_buffers.fifo_buffer.head = 0;

    state->audio_buffers.big_fifo_buffer.buffer = (float*)malloc(BIG_FIFO_BUFFER_SIZE * 2 * sizeof(float)); // times 2 because we have 2 channels
    state->audio_buffers.big_fifo_buffer.buffer_size = BIG_FIFO_BUFFER_SIZE * 2;
    state->audio_buffers.big_fifo_buffer.buffer_index = 0;
    state->audio_buffers.big_fifo_buffer.tail = 0;
    state->audio_buffers.big_fifo_buffer.head = 0;

    if (state->audio_buffers.main_buffer.buffer == NULL || state->audio_buffers.buffers == NULL || state->audio_buffers.fifo_buffer.buffer == NULL || state->audio_buffers.big_fifo_buffer.buffer == NULL) {
        printf("Failed to allocate audio buffers\n");
        exit(1);
    }
}

void update_audio_stream(AudioState *state) {
    // Lock the processing mutex
    mtx_lock(&state->processing_mutex);

    // Copy over the buffer[0] to the main buffer
    memcpy(state->audio_buffers.main_buffer.buffer, state->audio_buffers.buffers[0].buffer, BUFFER_SIZE * 2 * sizeof(float));

    // Now shift all the buffers
    for (int i = 0; i < AUDIO_BUFFER_COUNT - 1; i++) {
        memcpy(state->audio_buffers.buffers[i].buffer, state->audio_buffers.buffers[i + 1].buffer, BUFFER_SIZE * 2 * sizeof(float));
    }

    // Decrement the buffer index
    state->audio_buffers.buffer_index--;

    // Now we unlock the mutex, this is so uhhh something something idfkk prevents clicks
    mtx_unlock(&state->processing_mutex);

    // Copy the buffer from the main buffer to the audio stream
    UpdateAudioStream(stream, state->audio_buffers.main_buffer.buffer, BUFFER_SIZE);

    // Also update the fifo buffers
    fifo_audio_write(&state->audio_buffers.fifo_buffer, state->audio_buffers.main_buffer.buffer, BUFFER_SIZE * 2);
    fifo_audio_write(&state->audio_buffers.big_fifo_buffer, state->audio_buffers.main_buffer.buffer, BUFFER_SIZE * 2);

    printf("swapped buffer\n");
}

int AudioPlaybackThread(void* arg) {
    AudioState* state = (AudioState*)arg;

    while (state->running) {

        if (state->paused) {
            continue;
        }

        mtx_lock(&state->playback_mutex);

        if (IsAudioStreamProcessed(stream)) {
            update_audio_stream(state);
        }

        mtx_unlock(&state->playback_mutex);
    }
    return 0;
}

void reset_audiobuffers(void) {
    // Lock the processing mutex, and reset the buffer index
    mtx_lock(&audio_state.processing_mutex);
    audio_state.audio_buffers.buffer_index = 0;
    mtx_unlock(&audio_state.processing_mutex);
}

int AudioProcessingThread(void *audio_state) {
    AudioState *state = (AudioState *)audio_state;

    // Very similar to the AudioPlaybackThread, but we calculate the buffer here, the above one just uploads it to the audio stream
    while (state->running) {
        if (state->reset) {
            reset_audiobuffers();
            continue;
        }

        if (state->paused) {
            continue;
        }

        // We do NOT render directly to main buffer, we render to the current buffer, then we swap it with the main buffer
        if (state->audio_buffers.buffer_index >= AUDIO_BUFFER_COUNT)
            continue; // Idle out, until the audio playback thread catches up

        mtx_lock(&state->processing_mutex);

        // Render to the current buffer
        CalculateBuffer(state->audio_buffers.buffers[state->audio_buffers.buffer_index].buffer, BUFFER_SIZE);
        state->audio_buffers.buffer_index++;

        mtx_unlock(&state->processing_mutex);
    }

    return 0;
}

void play(__unused void* userdata) {
    // mark userdata as optional, so the warning does not show up
    mtx_lock(&audio_state.playback_mutex);
    audio_state.paused = false;
    mtx_unlock(&audio_state.playback_mutex);
}

void pauseCallback(__unused void* userdata) {
    mtx_lock(&audio_state.playback_mutex);
    audio_state.paused = true;
    mtx_unlock(&audio_state.playback_mutex);
}

void stop(__unused void* userdata) {
    mtx_lock(&audio_state.playback_mutex);
    audio_state.paused = true;
    audio_state.reset = true;
    mtx_unlock(&audio_state.playback_mutex);
}

void InitAudio() {
    // This initializes the audio state and threads
    InitAudioDevice();
    SetAudioStreamBufferSizeDefault(BUFFER_SIZE);
    stream = LoadAudioStream(SAMPLE_RATE, 32, 2);
    PlayAudioStream(stream);

    audio_state = (AudioState) {
        .running = true,
        .paused = true,
        .reset = false,
        .generator_state = generator_init(64), // cpu issues will probably happen around #500 generators
        .master_pan = 0,
        .master_volume = 1
    };

    create_buffers(&audio_state);

    //
    generator_add(&audio_state.generator_state, (Generator) {
        .frequency = 220,
        .phase = 0,
        .waveform = SINE,
        .amplitude = 0.5,
        .generate = GenerateWaveform,
        .panning = -1
    });

    mtx_init(&audio_state.playback_mutex, mtx_plain);
    mtx_init(&audio_state.processing_mutex, mtx_plain);

    audio_state.running = true;
    audio_state.paused = true;

    PlayCallback = play;
    PauseCallback = pauseCallback;
    StopCallback = stop;

    thrd_create(&audio_playback_thread, AudioPlaybackThread, &audio_state);
    thrd_create(&audio_process_thread, AudioProcessingThread, &audio_state);

    printf("Created audio thread\n");
}
