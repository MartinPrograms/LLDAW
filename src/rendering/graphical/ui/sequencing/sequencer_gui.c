#include "sequencer_gui.h"

void draw_sequencer_settings(){
    if (sequencer.settings.tracker) {
        CLAY_TEXT(CLAY_STRING("Tracker Mode"), CLAY_TEXT_CONFIG(
                {.textColor = COLOR_SCHEME_TEXT, .fontSize = 16, .letterSpacing = 0, .wrapMode = CLAY_TEXT_WRAP_NONE}));
    }else {
        CLAY_TEXT(CLAY_STRING("Sequencer Mode"), CLAY_TEXT_CONFIG(
                {.textColor = COLOR_SCHEME_TEXT, .fontSize = 16, .letterSpacing = 0, .wrapMode = CLAY_TEXT_WRAP_NONE}));
    }
}