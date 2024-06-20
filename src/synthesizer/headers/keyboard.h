#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <SDL2/SDL.h>

#define KEYBOARD_LENGTH sizeof(keyboard) / sizeof(keyboard[0])

const SDL_Keycode *keyboard;
bool *pressed;

int keyboard_find(SDL_Keycode sym);

void set_pressed(int index, bool value);

#endif
