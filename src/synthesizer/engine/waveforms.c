#include <math.h>
#include <stdlib.h>
#include "../headers/envelope.h"
#include "../headers/format.h"
#include "../headers/keyboard.h"
#include "../headers/waveforms.h"

#define MAX_OCTAVE 3
#define MIN_OCTAVE (-2)

#define NUMBER_OF_OSCILLATORS (SAWTOOTH + 1)

#define OCTAVE_STEP(step) pow(2, step)
#define SEMITONE_STEP(step) pow(2, (double) step / 12.0)

static float volume = 0.2;
static int octave = 0;
static float high_threshold = 240;
static float low_threshold = 200;

void volume_set(float value) {
    volume = value;
}

float get_volume(void) {
    return volume;
}

void octave_adjust(bool up) {
    bool isMax = octave == MAX_OCTAVE;
    bool isMin = octave == MIN_OCTAVE;
    if (up && !isMax) {
        octave++;
    } else if (!up && !isMin) {
        octave--;
    }
    if (isMax) {
        printf("Octave is MAX!\n");
    } else if (isMin) {
        printf("Octave is MIN!\n");
    }

    printf("Octave: %d\n", octave);
}

OscillatorType current_oscillator = SINE;

void oscillator_adjust(bool up) {
    bool isMax = current_oscillator == (NUMBER_OF_OSCILLATORS - 1);
    bool isMin = current_oscillator == SINE;
    if (up && !isMax) {
        current_oscillator++;
    } else if (!up && !isMin) {
        current_oscillator--;
    }

    if (isMax || isMin) {
        printf("No more oscillators!\n");
    }

    char *oscillator_string;
    switch (current_oscillator) {
        case SINE:
            oscillator_string = "sine";
            break;
        case TRIANGLE:
            oscillator_string = "triangle";
            break;
        case SQUARE:
            oscillator_string = "square";
            break;
        case SAWTOOTH:
            oscillator_string = "sawtooth";
            break;
    }
    printf("Oscillator: %s\n", oscillator_string);
}

static int phase = 0; // Global so that is continuous over calls. 
static int acc = 0;
static float lambda = 0.01;

int get_phase(void) {
    return phase;
}

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

// Implement this later.
float randomNoise(float phase, float freq) {
    return 0;
}

FilterType current_filter = NONE;

void filter_adjust(bool up) {
    bool isMax = current_filter == BAND_PASS;
    bool isMin = current_filter == NONE;
    if (up && !isMax) {
        current_filter++;
    } else if (!up && !isMin) {
        current_filter--;
    }

    if (isMax || isMin) {
        printf("No more filters!\n");
    }

    char *filter_string;
    switch (current_filter) {
        case SINE:
            filter_string = "none";
            break;
        case TRIANGLE:
            filter_string = "low pass";
            break;
        case SQUARE:
            filter_string = "high pass";
            break;
        case SAWTOOTH:
            filter_string = "band pass";
            break;
    }
    printf("Filter: %s\n", filter_string);
}

bool none(float freq) {
    return true;
}

bool low_pass(float freq) {
    return (freq < low_threshold);
}

bool high_pass(float freq) {
    return (freq > high_threshold);
}

bool band_pass(float freq) {
    return (!low_pass(freq) && !high_pass(freq));
}

void oscillatorCallback(void *userdata, Uint8 *stream, int len) {
    wave_function func;

    filter_function filter_func;

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

    switch (current_filter) {
        case NONE:
            filter_func = &none;
            break;
        case LOW_PASS:
            filter_func = &low_pass;
            break;
        case HIGH_PASS:
            filter_func = &high_pass;
            break;
        case BAND_PASS:
            filter_func = &band_pass;
            break;
    }

    float sample;
    for (int i = 0; i < len; i++) {
        stream[i] = 0;
        for (int j = 0; j < keyboard_length; j++) {
            float altered_freq = BASE_FREQ * OCTAVE_STEP(octave) * SEMITONE_STEP(j);
            sample = (*func)(phase, altered_freq) * // Get wave amplitude.
                get_amplitude(phase, j) * // Apply envelope.
                volume * DYNAMIC_RANGE; // Apply volume and dynamic range.
            
            if ((*filter_func)(altered_freq)) {
                stream[i] += sample;
            }
        }
        phase++;
    }
}
