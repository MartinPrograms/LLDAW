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
        points[i].y = centerY + buffer[i] * centerY;
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
    // Initialize kiss_fft
    int fftSize = bufferSize;
    kiss_fft_cfg cfg = kiss_fft_alloc(fftSize, 0, NULL, NULL);
    kiss_fft_cpx fftInput[fftSize];
    kiss_fft_cpx fftOutput[fftSize];

    // Prepare input data for FFT
    for (int i = 0; i < fftSize; i++) {
        fftInput[i].r = buffer[i]; // Real part (audio samples)
        fftInput[i].i = 0;        // Imaginary part (0 for real input)
    }

    // Perform FFT
    kiss_fft(cfg, fftInput, fftOutput);

    // Free FFT configuration
    kiss_fft_free(cfg);

    // Compute magnitudes and find the maximum magnitude for normalization
    float magnitudes[fftSize / 2 + 1];
    float maxMagnitude = 0.0f;
    for (int i = 0; i <= fftSize / 2; i++) {
        magnitudes[i] = sqrtf(fftOutput[i].r * fftOutput[i].r + fftOutput[i].i * fftOutput[i].i);
        if (magnitudes[i] > maxMagnitude) {
            maxMagnitude = magnitudes[i];
        }
    }

    // Normalize magnitudes to fit within the height
    if (maxMagnitude > 0) {
        for (int i = 0; i <= fftSize / 2; i++) {
            magnitudes[i] /= maxMagnitude;
        }
    }

    // Logarithmic frequency mapping
    float minFreq = 20.0f;   // Minimum frequency (20 Hz)
    float maxFreq = sampleRate / 2.0f; // Maximum frequency (Nyquist frequency)
    float logMin = logf(minFreq);
    float logMax = logf(maxFreq);

    // Draw the frequency spectrum as a smooth, connected line
    rlBegin(RL_LINES);
    rlSetLineWidth(2);
    rlColor4ub(color.r, color.g, color.b, color.a);

    for (int i = 0; i < quality; i++) {
        // Calculate the logarithmic frequency for this point
        float t = (float)i / (quality - 1); // Normalized position [0, 1]
        float logFreq = logMin + t * (logMax - logMin); // Logarithmic frequency
        float freq = expf(logFreq); // Convert back to linear frequency

        // Find the corresponding FFT bin for this frequency
        float bin = freq * fftSize / sampleRate;
        int bin1 = (int)bin; // Lower bin index
        int bin2 = bin1 + 1; // Upper bin index

        // Clamp bin indices to valid range
        if (bin1 < 0) bin1 = 0;
        if (bin1 > fftSize / 2) bin1 = fftSize / 2;
        if (bin2 < 0) bin2 = 0;
        if (bin2 > fftSize / 2) bin2 = fftSize / 2;

        // Interpolate magnitude between bins
        float fraction = bin - bin1; // Fractional part for interpolation
        float magnitude = magnitudes[bin1] * (1 - fraction) + magnitudes[bin2] * fraction;

        // Map to screen coordinates
        float screenX = x + t * width; // Logarithmic spacing
        float screenY = y + height - magnitude * height;

        // Draw a line to the next point (if not the last point)
        if (i < quality - 1) {
            float nextT = (float)(i + 1) / (quality - 1);
            float nextLogFreq = logMin + nextT * (logMax - logMin);
            float nextFreq = expf(nextLogFreq);
            float nextBin = nextFreq * fftSize / sampleRate;
            int nextBin1 = (int)nextBin;
            int nextBin2 = nextBin1 + 1;

            // Clamp next bin indices to valid range
            if (nextBin1 < 0) nextBin1 = 0;
            if (nextBin1 > fftSize / 2) nextBin1 = fftSize / 2;
            if (nextBin2 < 0) nextBin2 = 0;
            if (nextBin2 > fftSize / 2) nextBin2 = fftSize / 2;

            // Interpolate next magnitude
            float nextFraction = nextBin - nextBin1;
            float nextMagnitude = magnitudes[nextBin1] * (1 - nextFraction) + magnitudes[nextBin2] * nextFraction;

            // Map next point to screen coordinates
            float nextScreenX = x + nextT * width;
            float nextScreenY = y + height - nextMagnitude * height;

            // Draw a line between the current and next point
            rlVertex2f(screenX, screenY);
            rlVertex2f(nextScreenX, nextScreenY);
        }
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
    }
}

