#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <stdbool.h>
#include <SDL2/SDL.h>

typedef struct {
    float trigger_on_time;
    float trigger_off_time;
    bool note_on;
} note;

const int keyboard_length;

int keyboard_find(SDL_Keycode sym);

bool get_note_on(int index);

void set_note_on(int index, bool value);

float get_trigger_on_time(int index);

void set_trigger_on_time(int index, float phase);

float get_trigger_off_time(int index);

void set_trigger_off_time(int index, float phase);

#endif
