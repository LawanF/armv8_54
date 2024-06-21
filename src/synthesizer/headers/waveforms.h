#ifndef WAVEFORMS_H
#define WAVEFORMS_H

#include <SDL2/SDL.h>

typedef enum {SINE, TRIANGLE, SQUARE, SAWTOOTH} OscillatorType;

typedef enum { NONE, LOW_PASS, HIGH_PASS, BAND_PASS } FilterType;

FilterType current_filter;

bool none(float freq);

bool low_pass(float freq);

bool high_pass(float freq);

bool band_pass(float freq);

typedef bool (*filter_function)(float);

void filter_adjust(bool up);

float get_volume(void);

void volume_set(float step);

void octave_adjust(bool up);

OscillatorType current_oscillator;

void oscillator_adjust(bool up);

typedef float (*wave_function)(float, float);

int get_phase(void);

float fsin(float phase, float freq);

float square(float phase, float freq);

float triangle(float phase, float freq);

float sawtooth(float phase, float freq);

void oscillatorCallback(void *userdata, Uint8 *stream, int len);

#endif
