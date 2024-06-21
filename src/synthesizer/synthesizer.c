#include <SDL2/SDL.h>
#include <math.h>
#include <stdbool.h>
#include "headers/format.h"
#include "headers/keyboard.h"
#include "headers/settings.h"
#include "headers/waveforms.h"

// Macros for the window.
#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 480

int main(void) {
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
    want.format = AUDIO_FORMAT;
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
    SDL_PauseAudioDevice(device_id, 0); // Turn on the device.

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
                printf("Key press: %s!\n", SDL_GetKeyName(sym));

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

    // Free everything properly.
    SDL_DestroyWindow(window);
    SDL_CloseAudioDevice(device_id);
    SDL_Quit();

    return 0;
}
