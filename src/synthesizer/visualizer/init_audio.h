#ifndef INIT_AUDIO_H
#define INIT_AUDIO_H

#include <SDL2/SDL.h>

SDL_Window *get_audio_window(void);

void init_audio(void);

void set_volume(float value);

#endif
