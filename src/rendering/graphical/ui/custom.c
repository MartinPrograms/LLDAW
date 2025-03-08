#include <raylib.h>
#include <rlgl.h>
#include "custom.h"
#include "ui_settings.h"
#include "kiss_fft.h"
#include <math.h>
#include "../../audio/audio_state.h"

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
    float maxMagnitude = 0.0f;
    for (int i = 0; i <= halfSize; i++) {
        magnitudes[i] = sqrtf(fftOutput[i].r * fftOutput[i].r + fftOutput[i].i * fftOutput[i].i);
        if (magnitudes[i] > maxMagnitude) {
            maxMagnitude = magnitudes[i];
        }
    }
    if (maxMagnitude > 0) {
        for (int i = 0; i <= halfSize; i++) {
            magnitudes[i] /= maxMagnitude;
        }
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
    float totalTime = envelope.attack + envelope.decay + envelope.release;

    // Calculate x positions for each phase:
    float attackX = x + (envelope.attack / totalTime) * width;
    float decayX = x + ((envelope.attack + envelope.decay) / totalTime) * width;
    float endX   = x + width;

    // Assuming a typical screen coordinate system where y increases downward:
    // We want amplitude 1 at the top and amplitude 0 at the bottom.
    float startY   = y + height;                  // Amplitude 0
    float peakY    = y;                           // Amplitude 1 (attack peak)
    // Sustain level: interpolate between peak and start based on sustain value.
    float sustainY = y + height - envelope.sustain * height;

    // Create control points for the ADSR envelope.
    // 0: Start (0 amplitude)
    // 1: End of attack (peak at amplitude 1)
    // 2: End of decay (sustain level)
    // 3: End of release (back to 0)
    Vector2 points[4];
    points[0] = (Vector2){ x,        startY   };
    points[1] = (Vector2){ attackX,  peakY    };
    points[2] = (Vector2){ decayX,   sustainY };
    points[3] = (Vector2){ endX,     startY   };

    // Draw lines connecting the control points.
    rlBegin(RL_LINES);
    rlSetLineWidth(2);
    rlColor4ub(color.r, color.g, color.b, color.a);
    for (int i = 0; i < 3; i++) {
        rlVertex2f(points[i].x, points[i].y);
        rlVertex2f(points[i+1].x, points[i+1].y);
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

