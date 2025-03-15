#include <string.h>
#include "audio_processor.h"

#include "midi_processor.h"
#include "midi_state.h"
#include "../../helpers/audio/audio_math.h"

void CalculateBuffer(float *buffer, int bufferSize) {
    for (int i = 0; i < bufferSize; i++) {
        float sumL = 0;
        float sumR = 0;

        int noteCount = 0;
        Note* notes = sequencer_get_notes_at(audio_state.sample_number, &noteCount, buffer_arena);

        __unused MidiMessage *messages = NULL;
        int readout = 0;

        if (midi_state.enabled) {
            PmEvent event[4];
            messages = midi_processor_process(event, 4, &readout, buffer_arena);
        }

        for (int j = 0; j < audio_state.generator_state.generatorCount; j++) {
            Generator *generator = &audio_state.generator_state.generators[j];

            for (int k = 0; k < noteCount; k++) {
                Note note = notes[k];
                if (note.generator_index == j) {
                    if (note.last_sample == audio_state.sample_number) {
                        generator_voice_process(note.midi_note, note.frequency, note.amplitude, note.pan, true, generator);
                    }
                    else if (note.first_sample == audio_state.sample_number) {
                        generator_voice_process(note.midi_note, note.frequency, note.amplitude, note.pan, false, generator);
                    }
                }


            }

            /*
            if (midi_state.enabled) {
                for (int k = 0; k < readout; k++) {
                    if (messages[k].status == 144){ // The status 144 is a note on message
                        // Set the velocity for the first generator to the velocity of the midi message. The midi message's velocity is 0-127, so we need to normalize it to 0-1
                        float amp = (float)messages[k].data2 / 127.0f;
                        float freq = midi_note_to_frequency((int)messages[k].data1);
                        bool remove = messages[k].data2 == 0; // If the velocity is 0, we remove the voice (cant use the amp as it is a floating point number, causes some weird bugs)

                        // Now we call the generator_voice function to generate the sound
                        generator_voice_process((int)messages[k].data1, freq, amp, remove, generator); // the note number is the frequency, and also the index of the voice (kind of, whatever)
                    }
                }
            }
            */

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

        audio_state.sample_number += 1;
        float pos = time_from_samples(audio_state.sample_number, SAMPLE_RATE);
        // Pos to beats
        audio_state.current_beat = pos / (60.0f / sequencer.settings.bpm);
    }
}

void create_buffers(AudioState *state, ARENA* arena) {
    state->audio_buffers = (AudioBuffers){0};

    state->audio_buffers.main_buffer.buffer = (float*)arena_alloc(arena,BUFFER_SIZE * 2 * sizeof(float)); // times 2 because we have 2 channels
    state->audio_buffers.main_buffer.buffer_size = BUFFER_SIZE * 2;

    state->audio_buffers.buffers = (AudioBuffer*)arena_alloc(arena,AUDIO_BUFFER_COUNT * sizeof(AudioBuffer));
    for (int i = 0; i < AUDIO_BUFFER_COUNT; i++) {
        state->audio_buffers.buffers[i].buffer = (float*)arena_alloc(arena,BUFFER_SIZE * 2 * sizeof(float)); // times 2 because we have 2 channels
        state->audio_buffers.buffers[i].buffer_size = BUFFER_SIZE * 2;
    }

    state->audio_buffers.fifo_buffer.buffer = (float*)arena_alloc(arena,BUFFER_SIZE * 2 * sizeof(float)); // times 2 because we have 2 channels
    state->audio_buffers.fifo_buffer.buffer_size = BUFFER_SIZE * 2;
    state->audio_buffers.fifo_buffer.buffer_index = 0;
    state->audio_buffers.fifo_buffer.tail = 0;
    state->audio_buffers.fifo_buffer.head = 0;

    state->audio_buffers.big_fifo_buffer.buffer = (float*)arena_alloc(arena,BIG_FIFO_BUFFER_SIZE * 2 * sizeof(float)); // times 2 because we have 2 channels
    state->audio_buffers.big_fifo_buffer.buffer_size = BIG_FIFO_BUFFER_SIZE * 2;
    state->audio_buffers.big_fifo_buffer.buffer_index = 0;
    state->audio_buffers.big_fifo_buffer.tail = 0;
    state->audio_buffers.big_fifo_buffer.head = 0;

    state->current_beat = 0.0f;
    state->sample_number = 0;

    if (state->audio_buffers.main_buffer.buffer == NULL || state->audio_buffers.buffers == NULL || state->audio_buffers.fifo_buffer.buffer == NULL || state->audio_buffers.big_fifo_buffer.buffer == NULL) {
        printf("Failed to allocate audio buffers\n");
        exit(1);
    }
}

void update_audio_stream(AudioState *state) {
    if (state->audio_buffers.buffer_count == 0){
        printf("[WARNING] BUFFER UNDER-RUN 0\n");
        return;
    }

    int index = state->audio_buffers.buffer_head;
    // Copy the buffer and adjust indices under mutex protection
    memcpy(state->audio_buffers.main_buffer.buffer, state->audio_buffers.buffers[index].buffer, state->audio_buffers.buffers[index].buffer_size * sizeof(float));

    state->audio_buffers.buffer_head = (index + 1) % AUDIO_BUFFER_COUNT;
    state->audio_buffers.buffer_count--;

    UpdateAudioStream(stream, state->audio_buffers.main_buffer.buffer, BUFFER_SIZE);

    // Add the updated buffer to both fifo buffers
    fifo_audio_write(&state->audio_buffers.fifo_buffer, state->audio_buffers.main_buffer.buffer, BUFFER_SIZE * 2);
    fifo_audio_write(&state->audio_buffers.big_fifo_buffer, state->audio_buffers.main_buffer.buffer, BUFFER_SIZE * 2);
}

int AudioPlaybackThread(void* arg) {
    AudioState* state = (AudioState*)arg;
    while (state->running) {
        mtx_lock(&state->processing_mutex);
        while (state->paused && state->running) {
            cnd_wait(&state->resume_playback_cnd, &state->processing_mutex);
        }
        mtx_unlock(&state->processing_mutex);

        if (IsAudioStreamProcessed(stream)) {
            update_audio_stream(state);
        }
    }

    return 0;
}

void reset_audiobuffers(void) {
    mtx_lock(&audio_state.processing_mutex);
    // Clear all buffers to silence
    for (int i = 0; i < AUDIO_BUFFER_COUNT; i++) {
        memset(audio_state.audio_buffers.buffers[i].buffer, 0,
               BUFFER_SIZE * 2 * sizeof(float));
    }
    audio_state.audio_buffers.buffer_count = 0;
    audio_state.current_beat = 0.0f;
    audio_state.sample_number = 0;
    mtx_unlock(&audio_state.processing_mutex);
}

int AudioProcessingThread(void *arg) {
    AudioState *state = (AudioState *)arg;
    while (state->running) {
        mtx_lock(&state->processing_mutex);
        while (state->paused && state->running) {
            cnd_wait(&state->resume_processing_cnd, &state->processing_mutex);
            audio_state.audio_buffers.buffer_count = 0;
            audio_state.audio_buffers.buffer_head = 0;
            audio_state.audio_buffers.buffer_tail = 0;
        }

        if (state->audio_buffers.buffer_count >= AUDIO_BUFFER_COUNT) {
            mtx_unlock(&state->processing_mutex);
            continue;
        }

        if (state->started && state->audio_buffers.buffer_count == AUDIO_BUFFER_COUNT - 1) {
            state->started = false;
            cnd_signal(&state->resume_playback_cnd);
        }

        if (state->reset) {
            // This means we clear all the buffers
            for (int i = 0; i < AUDIO_BUFFER_COUNT; i++) {
                memset(state->audio_buffers.buffers[i].buffer, 0, BUFFER_SIZE * 2 * sizeof(float));
            }

            memset(state->audio_buffers.main_buffer.buffer, 0, BUFFER_SIZE * sizeof(float));

            state->audio_buffers.buffer_count = 0;
            state->audio_buffers.buffer_head = 0;
            state->audio_buffers.buffer_tail = 0;
            state->reset = false;
            state->current_beat = 0.0f;
            state->sample_number = 0;

            // Go through all the notes and KILL THEM ALL
            for (int i = 0; i < state->generator_state.generatorCount; i++) {
                generator_kill_all_voices(&state->generator_state.generators[i]);
            }
        }

        int64_t start = get_time_now(); // In seconds

        arena_reset(buffer_arena); // Clear the buffer arena (remove old midi messages)
        CalculateBuffer(state->audio_buffers.buffers[state->audio_buffers.buffer_tail].buffer, BUFFER_SIZE);

        int64_t end = get_time_now(); // In seconds

        state->time_to_render_buffer = nanoseconds_to_milliseconds(end - start);
        state->audio_buffers.buffer_tail = (state->audio_buffers.buffer_tail + 1) % AUDIO_BUFFER_COUNT;
        state->audio_buffers.buffer_count++;

        mtx_unlock(&state->processing_mutex);
    }
    return 0;
}

void play(__unused void* userdata) {
    mtx_lock(&audio_state.processing_mutex);
    audio_state.paused = false;
    audio_state.started = true;
    mtx_unlock(&audio_state.processing_mutex);
    cnd_signal(&audio_state.resume_processing_cnd);
}

void pauseCallback(__unused void* userdata) {
    mtx_lock(&audio_state.processing_mutex);
    audio_state.paused = true;
    mtx_unlock(&audio_state.processing_mutex);
}

void stop(__unused void* userdata) {
    mtx_lock(&audio_state.processing_mutex);
    audio_state.paused = true;
    audio_state.reset = true;
    audio_state.sample_number = 0;
    audio_state.current_beat = 0.0f;
    mtx_unlock(&audio_state.processing_mutex);
}

void InitAudio() {
    // Initialize audio device and stream as before
    InitAudioDevice();
    SetAudioStreamBufferSizeDefault(BUFFER_SIZE);
    stream = LoadAudioStream(SAMPLE_RATE, 32, 2);
    PlayAudioStream(stream);

    // Initialize the audio state
    audio_state = (AudioState) {
        .running = true,
        .paused = true,
        .reset = false,
        .generator_state = generator_init(64), // caution: many generators may hurt CPU (duh)
        .master_pan = 0,
        .master_volume = 1
    };

    create_buffers(&audio_state, default_arena); // long lasting memory

    // Initialize synchronization primitives
    mtx_init(&audio_state.state_mutex, mtx_plain);
    cnd_init(&audio_state.resume_processing_cnd);
    cnd_init(&audio_state.resume_playback_cnd);
    mtx_init(&audio_state.processing_mutex, mtx_plain);

    // Set callbacks
    PlayCallback = play;
    PauseCallback = pauseCallback;
    StopCallback = stop;

    // Just before launching the audio threads, create midi input
    midi_processor_init(false);

    // Create threads for playback and processing
    thrd_create(&audio_playback_thread, AudioPlaybackThread, &audio_state);
    thrd_create(&audio_process_thread, AudioProcessingThread, &audio_state);

    printf("Created audio threads\n");
    printf("Audio initialization complete\n");
}
