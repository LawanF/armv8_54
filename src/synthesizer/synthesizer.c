#include <SDL2/SDL.h>
#include <math.h>
#include <stdbool.h>
#include "headers/format.h"
#include "headers/keyboard.h"
#include "headers/settings.h"
#include "headers/waveforms.h"
#include "headers/audio_window.h"
#include "headers/event.h"
#include "headers/piano.h"
#include "headers/window.h"
#include "headers/constants.h"

int main(void) {
    // Initialise video and audio
    init_audio();
    // Main loop for polling events.
    bool isRunning = true;
    SDL_Event event;
    
    SDL_Keycode sym; // Symbol of key pressed.
    int index; // Index of key pressed.
    while (isRunning) {
        // Fetch the next event in the queue.
        while (SDL_PollEvent(&event) != 0) {
            // If it's a quit, terminate, otherwise, parse input.
            if (event.type == SDL_QUIT) {
                isRunning = false;
            } else if (event.type == SDL_KEYDOWN) {
                sym = event.key.keysym.sym; // Fetch key symbol.

                if (sym == SDLK_ESCAPE || sym == SDLK_SPACE) {
                    isRunning = false;
                    break;
                }

                // Adjust pressed flag if it's a keyboard key.
                index = keyboard_find(sym);
                if (index != -1) {
                    set_trigger_on_time(index, get_phase());
                    set_note_on(index, ON);
                    break;
                }

                // Switch case for settings.
                switch (sym) {
                    case SDLK_UP:
                        setting_adjust(true);
                        break;
                    case SDLK_DOWN:
                        setting_adjust(false);
                        break;
                    case SDLK_LEFT:
                        setting_left();
                        break;
                    case SDLK_RIGHT:
                        setting_right();
                        break;
                    default:
                        break;
                }
            } else if (event.type == SDL_KEYUP) {
                sym = event.key.keysym.sym; // Fetch key symbol.

                // Adjust keyboard pressed flag.
                index = keyboard_find(sym);
                if (index != -1) {
                    set_trigger_off_time(index, get_phase());
                    set_note_on(index, OFF);
                }
            }
        }
    }

    end_window();
    return 0;
}
