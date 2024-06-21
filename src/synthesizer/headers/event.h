#ifndef EVENT_H
#define EVENT_H

#include <SDL2/SDL.h>
#include <stdbool.h>

/**
 * Handles a key press or release in the main thread of the program.
 * @param event
 * @return -1 if the input was ignored, 0 if the program should be terminated,
 * and 1 otherwise
 */
int handle_event(SDL_Event event);

#endif
