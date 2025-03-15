#ifndef UI_SETTINGS_H
#define UI_SETTINGS_H

#include <math.h>

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
#define LARGE_FONT_SIZE 32
#define STANDARD_CORNER_RADIUS 4

// This header also includes some basic conversion functions for clay to raylib
#define CLAY_RECTANGLE_TO_RAYLIB_RECTANGLE(rectangle) (Rectangle) { .x = rectangle.x, .y = rectangle.y, .width = rectangle.width, .height = rectangle.height }
#define CLAY_COLOR_TO_RAYLIB_COLOR(color) (Color) { .r = (unsigned char)roundf(color.r), .g = (unsigned char)roundf(color.g), .b = (unsigned char)roundf(color.b), .a = (unsigned char)roundf(color.a) }

#endif // UI_SETTINGS_H