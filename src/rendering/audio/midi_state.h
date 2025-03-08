#pragma once
#ifndef MIDI_STATE_H
#define MIDI_STATE_H
#include <portmidi.h>

typedef struct {
    bool enabled; // If false, 0 attempts will be made to use any MIDI functionality
    bool paused; // If true, no MIDI events will be processed
    bool verbose; // If true, every MIDI event will be logged
    PmStream* midi_stream; // The MIDI stream
} MidiState;

typedef struct {
    // For now we'll only support piano esque MIDI input
    // We'll add more functionality later

    /// Midi message status (note on, note off, etc.)
    int status;
    int data1;
    int data2;
} MidiMessage;

extern MidiState midi_state;

#endif // MIDI_STATE_H