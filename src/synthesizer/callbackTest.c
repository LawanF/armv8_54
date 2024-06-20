#include <SDL2/SDL.h>
#include <math.h>
#include <stdbool.h>

// Macros for the window.
#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 480

// Macros for the format.
#define SAMPLE_RATE 48000
#define CHANNELS 1
#define BUFFER_SIZE 512
#define DYNAMIC_RANGE 127

// Macros for the keyboard.
#define KEYBOARD_LENGTH sizeof(keyboard) / sizeof(keyboard[0])

// Wave function.
typedef float (*wave_function)(float, float);

// Oscillator type.
typedef enum {SINE, TRIANGLE, SQUARE, SAWTOOTH} OscillatorType;

OscillatorType current_oscillator = SINE;
float base_freq = 220;
float wave_freq = 0; // Remove this later.

float volume = 0.2; // Volume.

// Array for keyboard layout.
const SDL_Keycode keyboard[] = {SDLK_a, SDLK_w, SDLK_s, SDLK_d, SDLK_r, SDLK_f, 
    SDLK_t, SDLK_g, SDLK_h, SDLK_u, SDLK_j, SDLK_i, SDLK_k, SDLK_o, SDLK_l, SDLK_SEMICOLON};
const int keyboard_length = sizeof(keyboard) / sizeof(keyboard[0]);

// Flags for which keys are pressed right now.
bool pressed[KEYBOARD_LENGTH] = {false};

void oscillatorCallback(void *userdata, Uint8 *stream, int len); // Prototype for AudioSpec.
int keyboard_find(SDL_Keycode sym); // Prototype for linear search over keyboard.
void set_pressed(int index, bool value);

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
    
    SDL_Keycode sym; // Symbol of key pressed.
    int index; // Index of the key pressed.
    while (isRunning) {
        // Fetch the next event in the queue.
        while (SDL_PollEvent(&event) != 0) {
            // If it's a quit, terminate, otherwise, parse input.
            if (event.type == SDL_QUIT) {
                isRunning = false;
            } else if (event.type == SDL_KEYDOWN) {
                sym = event.key.keysym.sym; // Fetch key symbol.
                printf("Key press: %s!\n", SDL_GetKeyName(sym));

                // Adjust pressed flag if it's a keyboard key.
                set_pressed(keyboard_find(sym), true);

                // Switch case for settings.
                switch (sym) {
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
                    case SDLK_UP:
                        volume += 0.01;
                    case SDLK_DOWN:
                        volume -= 0.01;
                        break;
                    default:
                        break;
                }
            } else if (event.type == SDL_KEYUP) {
                sym = event.key.keysym.sym; // Fetch key symbol.

                // Adjust keyboard flag.
                set_pressed(keyboard_find(sym), false);
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

float fsin(float phase, float freq) {
    return sin(2 * M_PI * (phase * freq / SAMPLE_RATE));
}

float square(float phase, float freq) {
    return sin(2 * M_PI * (phase * freq / SAMPLE_RATE)) > 0 ? 1 : -1;
}

float triangle(float phase, float freq) {
    return asin(sin(2 * M_PI * (phase * freq / SAMPLE_RATE))) * 2 / M_PI;
}

float sawtooth(float phase, float freq) {
    printf("phase: %f\n", phase);
    printf("sawtooth: %f\n", fmod(phase, 1.0));
    return (M_PI * freq * fmod(phase / SAMPLE_RATE, 1.0 / freq) - M_PI / 2) * 2 / M_PI;
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
        stream[i] = 0;
        for (int j = 0; j < keyboard_length; j++) {
            if (pressed[j]) {
                stream[i] += volume * DYNAMIC_RANGE * (*func)(phase, base_freq * pow(2, (float) j / 12.0));
                printf("stream: %d\n", stream[i]);
            }
        }
        phase++;
    }
}

// Utilities
int keyboard_find(SDL_Keycode sym) {
    for (int i = 0; i < keyboard_length; i++) {
        if (keyboard[i] == sym) {
            return i;
        }
    }
    return -1;
}

void set_pressed(int index, bool value) {
    if (index != -1) {
        pressed[index] = value;
    }
}
