#include "midi_processor.h"

#include <stdio.h>

void midi_processor_init(bool verbose) {
    midi_state.verbose = verbose;

    PmError err = Pm_Initialize();

    if (err != pmNoError) {
        printf("Error initializing PortMidi: %s\n", Pm_GetErrorText(err));
        midi_state.enabled = false;
        return;
    }

    int num_devices = Pm_CountDevices();
    if (num_devices == 0) {
        printf("No MIDI devices found\n");
        midi_state.enabled = false;
        return;
    }

    bool found_input = false;
    int input_device_id = -1;

    for (int i = 0; i < num_devices; i++) {
        const PmDeviceInfo *info = Pm_GetDeviceInfo(i);
        if (info->input) {
            found_input = true;
            input_device_id = i;
            printf("MIDI input device (is input: %i) %d: %s\n", i, info->input, info->name);
        }
    }

    // TODO: Add support for midi_output devices (so far we'll just use the first available input device)
    // TODO: Add support for multiple midi devices

    if (!found_input) {
        printf("No MIDI input devices found\n");
        midi_state.enabled = false;
        return;
    }

    midi_state.midi_stream = arena_alloc(default_arena, sizeof(PmStream));
    err = Pm_OpenInput(&midi_state.midi_stream, input_device_id, NULL, 1024, NULL, NULL);
    if (err != pmNoError) {
        printf("Error opening MIDI input: %s\n", Pm_GetErrorText(err));
        midi_state.enabled = false;
        return;
    }

    midi_state.enabled = true;

    printf("MIDI stream opened!\n");
}

MidiMessage* midi_processor_process(PmEvent *event, int count, int* readout, ARENA* arena) {
    MidiMessage* messages = (MidiMessage*)arena_alloc(arena, count * sizeof(MidiMessage));
    int read = Pm_Read(midi_state.midi_stream, event, count);

    // Output the readout
    *readout = read;

    if (read < 0) {
        printf("Error reading MIDI input: %s\n", Pm_GetErrorText(read));
        return NULL;
    }

    for (int i = 0; i < read; i++) {
        PmEvent e = event[i];

        // Deconstruct the MIDI message
        MidiMessage m = (MidiMessage){
            .status = e.message & 0xFF,
            .data1 = (e.message >> 8) & 0xFF,
            .data2 = (e.message >> 16) & 0xFF
        };

        messages[i] = m;

        if (midi_state.verbose) {
            printf("MIDI event: %d %d %d\n", e.message & 0xFF, (e.message >> 8) & 0xFF, (e.message >> 16) & 0xFF);
        }
    }

    return messages;
}

void midi_processor_close() {
    Pm_Close(&midi_state.midi_stream);
    Pm_Terminate();
}
