#include <raylib.h>
#include <rlgl.h>
#include "custom.h"

#include <clay.h>

#include "ui_settings.h"
#include "kiss_fft.h"
#include <math.h>

#include "components.h"
#include "../../../helpers/include.h"
#include "../../audio/audio_state.h"
#include "../../audio/generator.h"
#include "../../../helpers/audio/audio_definitions.h"
#include "../../../helpers/basic/string.h"

void WaveformDraw(const float* buffer, int bufferSize, Color color, float width, float height, float x, float y){
    int centerY = height / 2;
    Vector2 points[bufferSize];

    for (int i = 0; i < bufferSize; i++) {
        points[i] = (Vector2){0, 0};
        points[i].x = (float)i * width / bufferSize;

        float sample = buffer[i];
        if (sample > 1.0f) sample = 1.0f;
        if (sample < -1.0f) sample = -1.0f;
        points[i].y = centerY - sample * centerY;

        points[i].x += x;
        points[i].y += y;
    }

    rlBegin(RL_LINES);
    rlSetLineWidth(2);
    for (int i = 0; i < bufferSize - 1; i++) {
        rlColor4ub(color.r, color.g, color.b, color.a);
        rlVertex2f(points[i].x, points[i].y);
        rlVertex2f(points[i + 1].x, points[i + 1].y);
    }

    rlEnd();
}

// Function to compute the FFT and draw the frequency spectrum
void SpectrumDraw(const float* buffer, int bufferSize, Color color, float width, float height, float x, float y, float sampleRate, int quality) {
    // --- Compute FFT ---
    int fftSize = bufferSize;
    kiss_fft_cfg cfg = kiss_fft_alloc(fftSize, 0, NULL, NULL);
    kiss_fft_cpx fftInput[fftSize];
    kiss_fft_cpx fftOutput[fftSize];

    // Prepare input data for FFT (real-valued, imaginary = 0)
    for (int i = 0; i < fftSize; i++) {
        fftInput[i].r = buffer[i];
        fftInput[i].i = 0;
    }

    // Execute FFT
    kiss_fft(cfg, fftInput, fftOutput);
    kiss_fft_free(cfg);

    // --- Compute Magnitudes and Normalize ---
    int halfSize = fftSize / 2;
    float magnitudes[halfSize + 1];
    float maxMagnitude = 25;
    for (int i = 0; i <= halfSize; i++) {
        magnitudes[i] = sqrtf(fftOutput[i].r * fftOutput[i].r + fftOutput[i].i * fftOutput[i].i);

        if (magnitudes[i] > maxMagnitude && magnitudes[i] > 25) { // Prevent drawing noise
            maxMagnitude = magnitudes[i];
        }
    }

    // Normalize magnitudes
    for (int i = 0; i <= halfSize; i++) {
        magnitudes[i] /= maxMagnitude;
    }

    // --- Draw Vertical Lines with Logarithmic Frequency Mapping ---
    // We iterate over 'quality' steps. For each step, we compute a normalized
    // horizontal position (u in [0,1]) and then compute the corresponding frequency.
    // The mapping is piecewise:
    //  • For u in [0, 0.5]: f(u) = 20 * (440/20)^(2*u)
    //       so that f(0)=20 and f(0.5)=20*(440/20)=440.
    //  • For u in [0.5, 1]: let t = 2*u - 1, then:
    //       f(u) = 440 * (22500/440)^( t^(1/0.725) )
    //       so that f(0.5)=440 and f(1)=22500, and approximately f(0.75)=2000.
    rlBegin(RL_LINES);
    rlSetLineWidth(2);
    for (int i = 0; i < quality; i++) {
        float u = (float)i / (quality - 1); // normalized [0, 1]
        float f; // frequency corresponding to this u

        if (u < 0.5f) {
            // Lower segment: 20 Hz -> 440 Hz (u: 0 -> 0.5)
            float t = 2.0f * u;  // t goes from 0 to 1
            // f = 20 * (440/20)^t
            f = 20.0f * powf(440.0f / 20.0f, t);
        } else {
            // Upper segment: 440 Hz -> 22.5 kHz (u: 0.5 -> 1)
            float t = 2.0f * u - 1.0f;  // t goes from 0 to 1
            // Inverse of: x = 0.5 + 0.5 * pow((log(f)-log(440))/(log(22500)-log(440)), 0.725)
            // yields: f = 440 * (22500/440)^( t^(1/0.725) )
            float exponent = powf(t, 1.0f / 0.725f);
            f = 440.0f * powf(22500.0f / 440.0f, exponent);
        }

        // Clamp frequency to Nyquist (sampleRate/2)
        if (f > sampleRate / 2.0f) f = sampleRate / 2.0f;

        // Map frequency to FFT bin index:
        int bin = (int)(f * fftSize / sampleRate);
        if (bin > halfSize) bin = halfSize;

        // Compute the x position (evenly spaced along the width)
        float lineX = x + u * width;
        // Scale the magnitude to the drawing height
        float lineHeight = magnitudes[bin] * height;

        // Draw vertical line (from bottom up)
        rlColor4ub(color.r, color.g, color.b, color.a);
        rlVertex2f(lineX, y + height);               // Bottom of the line
        rlVertex2f(lineX, y + height - lineHeight);    // Top of the line
    }
    rlEnd();
}

void AdsrDraw(AdsrEnvelope envelope, Color color, float width, float height, float x, float y) {
    // Total time (in seconds) for the drawn envelope (attack, decay, release)
    float totalTime = envelope.attack.value + envelope.decay.value + envelope.release.value;
    float activeTime = envelope.attack.value + envelope.decay.value;
    int64_t active_samples = (int64_t)(activeTime * SAMPLE_RATE);

    float heights[(int)width]; // Height of the envelope at each x position
    for (int i = 0; i < width; i++) {
        // we need to get the sample number for this x position
        int64_t sample = (int64_t)(i * totalTime / width * SAMPLE_RATE);
        float value = adsr_from_cache(height, sample, 0, active_samples, envelope, active_samples < sample, NULL);
        heights[i] = value;
    }

    rlBegin(RL_LINES);
    rlSetLineWidth(2);

    for (int i = 0; i < width - 1; i++) {
        rlColor4ub(color.r, color.g, color.b, color.a);
        // Draw a line from the bottom of the envelope to the height at this x position
        rlVertex2f(x + i, y + height);
        rlVertex2f(x + i, y + height - heights[i]);
    }

    rlEnd();
}

void RenderCustomElement(CustomElement *element, float width, float height, float x, float y) {
    switch (element->type){
        case CUSTOM_ELEMENT_TYPE_WAVEFORM:
            WaveformDraw(element->customData.waveform.buffer, element->customData.waveform.bufferSize, element->color, width, height, x, y);
            break;

        case CUSTOM_ELEMENT_TYPE_SPECTRUM:
            SpectrumDraw(element->customData.spectrum.buffer, element->customData.spectrum.bufferSize, element->color, width, height, x, y, SAMPLE_RATE, element->customData.spectrum.quality);
            break;

        case CUSTOM_ELEMENT_TYPE_ADSR:
            AdsrDraw(element->customData.adsr.envelope, element->color, width, height, x, y);
            break;
    }
}

