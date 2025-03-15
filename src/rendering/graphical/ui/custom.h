#ifndef CUSTOM_H
#define CUSTOM_H

#include <raylib.h>
#include "../../audio/generator.h"

typedef enum {
    CUSTOM_ELEMENT_TYPE_WAVEFORM,
    CUSTOM_ELEMENT_TYPE_SPECTRUM,
    CUSTOM_ELEMENT_TYPE_ADSR,
} CustomElementType;

// This is a custom component library.
typedef struct {
    CustomElementType type;
    Color color;
    union {
        struct {
            const float *buffer;
            int bufferSize;
        } waveform;
        struct {
            const float *buffer;
            int bufferSize;
            int quality; // How many lines in the output
        } spectrum;
        struct {
            AdsrEnvelope envelope;
        } adsr;
    } customData;
} CustomElement;

void RenderCustomElement(CustomElement *element, float width, float height, float x, float y);

#endif // CUSTOM_H