#include <math.h>
#include "../headers/format.h"
#include "../headers/keyboard.h"
#include "../headers/waveforms.h"

static float volume = 0.2;

OscillatorType current_oscillator = SINE;

float fsin(float phase, float freq) {
    return sin(2 * M_PI * (phase * freq / SAMPLE_RATE));
}

float square(float phase, float freq) {
    return sin(2 * M_PI * (phase * freq / SAMPLE_RATE)) > 0 ? 1 : -1;
}

float triangle(float phase, float freq) {
    return asin(sin(2 * M_PI * (phase * freq / SAMPLE_RATE))) * 2 / M_PI;
}

float sawtooth(float phase, float freq) {
    return (M_PI * freq * fmod(phase / SAMPLE_RATE, 1.0 / freq) - M_PI / 2) * 2 / M_PI;
}

void oscillatorCallback(void *userdata, Uint8 *stream, int len) {
    static int phase = 0; // Static so that the wave is continuous over buffer calls.
    wave_function func;

    switch (current_oscillator) {
        case SINE:
            func = &fsin;
            break;
        case TRIANGLE:
            func = &triangle;
            break;
        case SQUARE:
            func = &square;
            break;
        case SAWTOOTH:
            func = &sawtooth;
            break;
    }
    for (int i = 0; i < len; i++) {
        stream[i] = 0;
        for (int j = 0; j < KEYBOARD_LENGTH; j++) {
            if (pressed[j]) {
                stream[i] += volume * DYNAMIC_RANGE * (*func)(phase, BASE_FREQ * pow(2, (float) j / 12.0));
                printf("stream: %d\n", stream[i]);
            }
        }
        phase++;
    }
}

void volume_add(float step) {
    volume += step;
}