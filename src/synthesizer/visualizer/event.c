#include <ncurses.h>
#include "event.h"
#include "init_audio.h"

int handle_event(SDL_Event event) {
    if (event.type == SDL_QUIT) return 0;
    SDL_Keycode key_code;
    switch (event.type) {
        case SDL_KEYUP:
        case SDL_KEYDOWN: {
            key_code = event.key.keysym.sym;
            if (key_code == SDLK_ESCAPE || key_code == SDLK_SPACE) return 0;
            if (key_code != SDLK_a) return -1;
            float new_volume = (event.type == SDL_KEYDOWN) ? 1 : 0;
            set_volume(new_volume);
            printw("New volume: %f\n", new_volume);
            return 1;
        }
        default: return -1;
    }
}

