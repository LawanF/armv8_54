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

#endif
