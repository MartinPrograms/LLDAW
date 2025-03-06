#ifndef BASE_UI_H
#define BASE_UI_H

#include <clay.h>
#include "../../../helpers/include.h"

// Defining a basic color scheme, old tracker software esque.
// Blue for background, white for text, black for backgrounds, green for waveforms, red for errors.
#define COLOR_SCHEME_BACKGROUND {20, 30, 50, 255}
#define COLOR_SCHEME_BACKGROUND_SECONDARY {30, 40, 60, 255}
#define COLOR_SCHEME_BACKGROUND_TERTIARY {40, 50, 70, 255}
#define COLOR_SCHEME_TEXT {255, 255, 255, 255}
#define COLOR_SCHEME_BACKGROUND_DARK {10, 10, 10, 255}
#define COLOR_SCHEME_WAVEFORM {0, 255, 0, 255}
#define COLOR_SCHEME_ERROR {255, 0, 0, 255}
#define COLOR_SCHEME_BUTTON {70, 60, 80, 255}
#define STANDARD_PADDING 8
#define STANDARD_GAP 4
#define STANDARD_FONT_SIZE 16
#define STANDARD_CORNER_RADIUS 4

void RenderMainUI();
void CreateButton(const char *text, void (*callback)());

Clay_String GetString(const char *string);

#endif // BASE_UI_H