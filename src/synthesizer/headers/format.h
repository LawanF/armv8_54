#ifndef FORMAT_H
#define FORMAT_H

#include <SDL2/SDL.h>

#define BASE_FREQ 220.0f
#define VOLUME_STEP 0.01f

// Macros for the format.
#define SAMPLE_RATE 44100
#define AUDIO_FORMAT AUDIO_S8 // Has to be one of SDL_AudioFormat
#define CHANNELS 1
#define BUFFER_SIZE 512 // Lower = less latency, but more computational power.
#define DYNAMIC_RANGE 127 // Note that this has to be restricted to the audio format.

#endif
