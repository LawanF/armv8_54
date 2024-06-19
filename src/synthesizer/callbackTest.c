#include <SDL2/SDL.h>
#include <math.h>
#include <stdbool.h>

#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 480

#define SAMPLE_RATE 44100
#define CHANNELS 1
#define BUFFER_SIZE 256
#define DYNAMIC_RANGE 127

// Wave function.
typedef float (*wave_function)(float);

// Oscillator type.
typedef enum {SINE, TRIANGLE, SQUARE, SAWTOOTH} OscillatorType;

OscillatorType current_oscillator = SINE;
float waveFreq = 440.0;

void oscillatorCallback(void *userdata, Uint8 *stream, int len);

int main() {
    // Initialise video and audio
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
        fprintf(stderr, "SDL_Init error: %s\n", SDL_GetError());
    }

    // Initialise a window.
    SDL_Window *window = NULL;
    window = SDL_CreateWindow("Synthesizer", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_HEIGHT, WINDOW_WIDTH, SDL_WINDOW_SHOWN);

    if (window == NULL) {
        fprintf(stderr, "SDL_CreateWindow error: %s\n", SDL_GetError());
    }

    // Specify audio format.
    SDL_AudioSpec want, have;
    SDL_memset(&want, 0, sizeof(want));
    want.freq = SAMPLE_RATE;
    want.format = AUDIO_S8;
    want.channels = CHANNELS;
    want.samples = BUFFER_SIZE;
    want.callback = oscillatorCallback;

    // Open the device.
    SDL_AudioDeviceID device_id = SDL_OpenAudioDevice(NULL, 0, &want, &have, 0);
    if (!device_id) {
        fprintf(stderr, "SDL_OpenAudioDevice error: %s\n", SDL_GetError());
    }

    // Surface the window and unpause the device.
    SDL_UpdateWindowSurface(window);
    SDL_PauseAudioDevice(device_id, 0);

    // Main loop for polling events.
    bool isRunning = true;
    SDL_Event event;
    while (isRunning) {
        // Fetch the next event in the queue.
        while (SDL_PollEvent(&event) != 0) {
            // If it's a quit, terminate, otherwise, parse input.
            if (event.type == SDL_QUIT) {
                isRunning = false;
            } else if (event.type == SDL_KEYDOWN) {
                switch (event.key.keysym.sym) {
                    case SDLK_1:
                        current_oscillator = SINE;
                        break;
                    case SDLK_2:
                        current_oscillator = TRIANGLE;
                        break;
                    case SDLK_3:
                        current_oscillator = SQUARE;
                        break;
                    case SDLK_4:
                        current_oscillator = SAWTOOTH;
                        break;
                    default:
                        break;
                }
            }
        }
    }

    // Free everything properly.
    SDL_DestroyWindow(window);
    SDL_CloseAudioDevice(device_id);
    SDL_Quit();

    return 0;
}

void sineWave(void *userdata, Uint8 *stream, int len) {
    static int phase = 0;
    for (int i = 0; i < len; i++) {
        phase++;
        stream[i] = 128 * sin(2 * M_PI * 440 * phase / 44100);
    }
}

float fsin(float phase) {
    return sin(2 * M_PI * waveFreq * (phase / SAMPLE_RATE));
}

float square(float phase) {
    return sin(2 * M_PI * waveFreq * (phase / SAMPLE_RATE)) > 0 ? 1 : -1;
}

float triangle(float phase) {
    return asin(sin(2 * M_PI * waveFreq * (phase / SAMPLE_RATE))) * 2 / M_PI;
}

float sawtooth(float phase) {
    return fmod(((phase / SAMPLE_RATE)-((1/waveFreq)/2)), (1/waveFreq))*waveFreq*2 - 1;
}

void oscillatorCallback(void *userdata, Uint8 *stream, int len) {
    static int phase = 0; // Static so that the wave is continuous over buffer calls.
    wave_function func;

    switch (current_oscillator) {
        case SINE:
            func = &fsin;
            break;
        case TRIANGLE:
            func = &triangle;
            break;
        case SQUARE:
            func = &square;
            break;
        case SAWTOOTH:
            func = &sawtooth;
            break;
    }
    for (int i = 0; i < len; i++) {
        stream[i] = DYNAMIC_RANGE * (*func)(phase);
        phase++;
    }
}
