#include "audio_math.h"
float sine_table[SINE_TABLE_SIZE];
void init_math() {
    for (int i = 0; i < SINE_TABLE_SIZE; i++) {
        sine_table[i] = sinf(2.0f * PI * i / SINE_TABLE_SIZE);
    }
}

float quick_sine(float phase)  {
    // Phase is already between 0 and 2PI
    int index = (int)(phase * SINE_TABLE_SIZE / (2.0f * PI));
    return sine_table[index];
}