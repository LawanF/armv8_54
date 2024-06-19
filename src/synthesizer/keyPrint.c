#include <SDL2/SDL.h>
#include <stdio.h>

/*
    A file to print key presses and releases, along with modifiers.
*/


/*
    Prints the modifiers given a SDL_Keymod.
*/
void printModifiers(SDL_Keymod mod) {
    printf("Modifiers: ");

    if (mod == KMOD_NONE) {
        printf("None\n");
        return;
    }

    // There is no convenient SDL_GetModName() that gives us
    // a string representation of a modifier, so we must do this.
    if(mod & KMOD_NUM) printf("NUMLOCK ");
    if(mod & KMOD_CAPS) printf("CAPSLOCK");
    if(mod & KMOD_LCTRL) printf("LCTRL ");
    if(mod & KMOD_RCTRL) printf("RCTRL ");
    if(mod & KMOD_RSHIFT) printf("RSHIFT ");
    if(mod & KMOD_LSHIFT) printf("LSHIFT ");
    if(mod & KMOD_RALT) printf("RALT ");
    if(mod & KMOD_LALT) printf("LALT ");
    if(mod & KMOD_CTRL) printf("CTRL ");
    if(mod & KMOD_SHIFT) printf("SHIFT ");
    if(mod & KMOD_ALT) printf("ALT ");
    printf( "\n" );
}

/*
    Prints the key pressed or released, along with the modifiers at the time
    of the event.
*/
void printKeyInfo(SDL_KeyboardEvent key) {
    if (key.type == SDL_KEYDOWN) {
        printf("Pressed key: ");
    } else {
        printf("Released key: ");
    }
    printf("%s\n", SDL_GetKeyName(key.keysym.sym));
    printModifiers(key.keysym.mod);
}

int main() {
    /* Initialise SDL */
    if(SDL_Init(SDL_INIT_VIDEO) < 0) {
        fprintf(stderr, "Could not initialise SDL: %s\n", SDL_GetError());
        exit(-1);
    }

    // Create a window.
    SDL_Window *window = NULL;
    window = SDL_CreateWindow("Test Window", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 500, 500, SDL_WINDOW_SHOWN);

    // Check for error.
    if(window == NULL) {
        fprintf(stderr, "Window creation error: %s\n", SDL_GetError());
    } 

    int isRunning = 1; 
    SDL_Event event; // Event data type to be polled into.
    while (isRunning) {
        SDL_UpdateWindowSurface(window);
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_KEYDOWN:
                case SDL_KEYUP:
                    printKeyInfo(event.key);
                    break;
                case SDL_QUIT:
                    isRunning = 0;
                    break;
                default:
                    break;
            }
        }
    }

    SDL_DestroyWindow(window); // Free the window.
    SDL_Quit();
    return 0;
}
