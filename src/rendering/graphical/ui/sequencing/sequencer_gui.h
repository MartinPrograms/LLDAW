#ifndef SEQUENCER_GUI_H
#define SEQUENCER_GUI_H

#include <clay.h>
#include "../../../../sequencing/sequencer.h" // For the sequencer state
#include "../ui_settings.h"

void draw_sequencer_settings();
void draw_sequencer(SequencerState *sequencer);

#endif // SEQUENCER_GUI_H