#ifndef CONSTANTS_H
#define CONSTANTS_H

#define PRINT_SDL_ERR(name) fprintf(stderr, "Error occurred on SDL action %s: %s\n", \
    name, SDL_GetError())

// Parameters for the (hidden) SDL window for recording input

#define WINDOW_WIDTH  480
#define WINDOW_HEIGHT 320

#endif
