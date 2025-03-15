#ifndef MIDI_PROCESSOR_H
#define MIDI_PROCESSOR_H

#include <portmidi.h>

#include "midi_state.h"
#include "../../helpers/include.h"

void midi_processor_init(bool verbose); // if verbose is true, it will log every message
MidiMessage* midi_processor_process(PmEvent *event, int count,int* readout, ARENA* arena);
void midi_processor_close();

#endif // MIDI_PROCESSOR_H