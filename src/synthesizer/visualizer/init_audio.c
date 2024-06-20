#include "init_audio.h"
#include "constants.h"
#include "../headers/keyboard.h"
#include "../headers/format.h"
#include "../headers/waveforms.h"

SDL_Window *audio_window = NULL;

static float volume = 1;

void default_callback(void *userdata, Uint8 *stream, int len) {
    static double phase = 0;

    for (int i = 0; i < len; i++) {
        stream[i] = 0;
        for (int j = 0; j < keyboard_length; j++) {
            if (get_pressed(j)) {
                stream[i] += volume * DYNAMIC_RANGE * (fsin)(phase, BASE_FREQ * pow(2, (float) j / 12.0));
                printf("stream: %d\n", stream[i]);
            }
        }
        phase++;
    }
}

void init_audio(void) {
    if (SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO) < 0) {
        PRINT_SDL_ERR("Initialise SDL");
        exit(EXIT_FAILURE);
    }
    audio_window = SDL_CreateWindow(
            /* title = */ "Visualizer",
            /* x = */ SDL_WINDOWPOS_CENTERED,
            /* y = */ SDL_WINDOWPOS_CENTERED,
            /* w = */ WINDOW_WIDTH,
            /* h = */ WINDOW_HEIGHT,
            /* flags = */ SDL_WINDOW_HIDDEN);
    if (audio_window == NULL) {
        PRINT_SDL_ERR("Create SDL audio engine");
        exit(EXIT_FAILURE);
    }
    SDL_UpdateWindowSurface(audio_window);
    // Specify audio format.
    SDL_AudioSpec want, have;
    SDL_memset(&want, 0, sizeof(want));
    want.freq = SAMPLE_RATE;
    want.format = AUDIO_FORMAT;
    want.channels = 1;
    want.samples = BUFFER_SIZE;
    want.callback = &default_callback;

    // Open the device.
    SDL_AudioDeviceID device_id = SDL_OpenAudioDevice(NULL, 0, &want, &have, 0);
    if (!device_id) {
        fprintf(stderr, "SDL_OpenAudioDevice error: %s\n", SDL_GetError());
    }
}
