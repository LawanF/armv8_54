#include <math.h>
#include "../headers/audio_window.h"
#include "../headers/constants.h"
#include "../headers/format.h"
#include "../headers/waveforms.h"

static SDL_Window *audio_window = NULL;

SDL_Window *get_audio_window(void) {
    return audio_window;
}

static float volume = 1;

float fsin(float phase, float freq) {
    return sin(2 * M_PI * (phase * freq / SAMPLE_RATE));
}

void default_callback(void *userdata, Uint8 *stream, int len) {
    static double phase = 0;

    for (int i = 0; i < len; i++) {
        stream[i] = volume * DYNAMIC_RANGE * (fsin)(phase, BASE_FREQ * pow(2, (float) 1 / 12.0));
        phase++;
    }
}

void init_audio(void) {
    if (SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO | SDL_INIT_EVENTS) < 0) {
        PRINT_SDL_ERR("Initialise SDL");
        exit(EXIT_FAILURE);
    }
    audio_window = SDL_CreateWindow(
            /* title = */ "Visualizer",
            /* x = */ SDL_WINDOWPOS_CENTERED,
            /* y = */ SDL_WINDOWPOS_CENTERED,
            /* w = */ 0,
            /* h = */ 0,
            /* flags = */ SDL_WINDOW_FULLSCREEN_DESKTOP);
    if (audio_window == NULL) {
        PRINT_SDL_ERR("Create SDL audio engine");
        exit(EXIT_FAILURE);
    }
    SDL_UpdateWindowSurface(audio_window);
    SDL_SetWindowOpacity(audio_window, 0);

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
        PRINT_SDL_ERR("Obtain free device");
        exit(EXIT_FAILURE);
    }
    SDL_PauseAudioDevice(device_id, 0);
}

void set_volume(float value) {
    volume = (float) fmin(1.0, fmax(0, value));
}
