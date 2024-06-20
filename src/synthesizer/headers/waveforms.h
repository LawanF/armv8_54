#ifndef WAVEFORMS_H
#define WAVEFORMS_H

#include <SDL2/SDL.h>

typedef enum {SINE, TRIANGLE, SQUARE, SAWTOOTH} OscillatorType;

OscillatorType current_oscillator;

typedef float (*wave_function)(float, float);

int get_phase(void);

float fsin(float phase, float freq);

float square(float phase, float freq);

float triangle(float phase, float freq);

float sawtooth(float phase, float freq);

void oscillatorCallback(void *userdata, Uint8 *stream, int len);

void volume_adjust(float step);

#endif
