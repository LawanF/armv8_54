#include "../headers/keyboard.h"

#define KEYBOARD_LENGTH sizeof(keyboard) / sizeof(keyboard[0])

// Array for keyboard layout.
const SDL_Keycode keyboard[] = {SDLK_z, SDLK_s, SDLK_x, SDLK_c, SDLK_f, SDLK_v, SDLK_g, SDLK_b, SDLK_n, SDLK_j, SDLK_m, SDLK_k, // First octave.
    SDLK_q, SDLK_2, SDLK_w, SDLK_e, SDLK_4, SDLK_r, SDLK_5, SDLK_t, SDLK_y, SDLK_7, SDLK_u, SDLK_8, // Second octave.
    SDLK_i}; // High note.

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
