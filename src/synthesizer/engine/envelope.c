#include <stdbool.h>
#include "../headers/envelope.h"
#include "../headers/keyboard.h"

struct adsr {   
    float attack_time;
    float decay_time;
    float release_time;

    float start_amplitude;
    float sustain_amplitude; 
};

struct adsr  _adsr = {2, 1, 5, 1.0, 0.8}; 
ADSR adsr = &_adsr;


// phase may loop around D:
float get_amplitude(float phase, int index);

