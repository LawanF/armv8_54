#include "../headers/keyboard.h"

#define KEYBOARD_LENGTH sizeof(keyboard) / sizeof(keyboard[0])

// Array for keyboard layout.
const SDL_Keycode keyboard[] = {SDLK_a, SDLK_w, SDLK_s, SDLK_d, SDLK_r, SDLK_f, 
    SDLK_t, SDLK_g, SDLK_h, SDLK_u, SDLK_j, SDLK_i, SDLK_k, SDLK_o, SDLK_l, SDLK_SEMICOLON};

// Storing key trigger on and off times.
note notes[KEYBOARD_LENGTH];

const int keyboard_length = KEYBOARD_LENGTH;

int keyboard_find(SDL_Keycode sym) {
    for (int i = 0; i < KEYBOARD_LENGTH; i++) {
        if (keyboard[i] == sym) {
            return i;
        }
    }
    return -1;
}

NoteState get_note_on(int index) {
    return notes[index].note_on;
}

void set_note_on(int index, NoteState value) {
    notes[index].note_on = value;
}

void set_trigger_on_time(int index, float phase) {
    // If key hasn't been pressed before, set it's trigger time.
    if (get_note_on(index) != ON) {
        notes[index].trigger_on_time = phase;
    }
}

float get_trigger_on_time(int index) {
    return notes[index].trigger_on_time;
}

void set_trigger_off_time(int index, float phase) {
    notes[index].trigger_off_time = phase;
}

float get_trigger_off_time(int index) {
    return notes[index].trigger_off_time;
}
