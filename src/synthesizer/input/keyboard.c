#include <stdbool.h>
#include "../headers/keyboard.h"

// Array for keyboard layout.
const SDL_Keycode keyboard[] = {SDLK_a, SDLK_w, SDLK_s, SDLK_d, SDLK_r, SDLK_f, 
    SDLK_t, SDLK_g, SDLK_h, SDLK_u, SDLK_j, SDLK_i, SDLK_k, SDLK_o, SDLK_l, SDLK_SEMICOLON};

// Flags for which keys are pressed right now.
bool pressed[KEYBOARD_LENGTH] = {false};

int keyboard_find(SDL_Keycode sym) {
    for (int i = 0; i < KEYBOARD_LENGTH; i++) {
        if (keyboard[i] == sym) {
            return i;
        }
    }
    return -1;
}

void set_pressed(int index, bool value) {
    if (index != -1) {
        pressed[index] = value;
    }
}