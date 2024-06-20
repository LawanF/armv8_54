#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <stdbool.h>
#include <SDL2/SDL.h>

const int keyboard_length;

int keyboard_find(SDL_Keycode sym);

bool get_pressed(int index);

void set_pressed(int index, bool value);

#endif
