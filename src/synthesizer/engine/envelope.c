#include <stdbool.h>
#include "../headers/envelope.h"
#include "../headers/format.h"
#include "../headers/keyboard.h"

#define MINIMUM_AMPLITUDE 0.001f

struct adsr {   
    float attack_time;
    float decay_time;
    float release_time;

    float start_amplitude;
    float sustain_amplitude; 
};

struct adsr  _adsr = {2.0, 2.0, 5.0, 1.0, 0.6}; 


void attack_adjust(float step) {
    _adsr.attack_time += step;
}

void decay_adjust(float step) {
    _adsr.decay_time += step;
}

void release_adjust(float step) {
    _adsr.release_time += step;
}

void start_adjust(float step) {
    _adsr.start_amplitude += step;
}

void sustain_adjust(float step) {
    _adsr.sustain_amplitude += step;
}

// phase may loop around D:
float get_ADS_amplitude(float phase, int index) {    
    float trigger_on_time = get_trigger_on_time(index);
    float lifetime = (phase - trigger_on_time) / SAMPLE_RATE;
    float res_amplitude;
    if (lifetime < _adsr.attack_time) {
        // ATTACK
        res_amplitude = _adsr.start_amplitude * (lifetime / _adsr.attack_time);
    } else if (lifetime < _adsr.attack_time + _adsr.decay_time) {
        // DECAY    
        res_amplitude = _adsr.sustain_amplitude - (_adsr.start_amplitude - _adsr.sustain_amplitude) * ((lifetime - _adsr.attack_time) / _adsr.decay_time);
    } else {
        res_amplitude = _adsr.sustain_amplitude;
    }
    return res_amplitude;
}

float get_amplitude(int index, float phase) {
    bool note_on = get_note_on(index);
    float trigger_on_time = get_trigger_on_time(index);
    float trigger_off_time; // Might not be defined just yet.
    float lifetime = (phase - trigger_on_time) / SAMPLE_RATE;
    float deathtime;
    float res_amplitude;
    if (note_on) {
        // ADS
        res_amplitude = get_ADS_amplitude(phase, index);
    } else {
        // RELEASE
        trigger_off_time = get_trigger_off_time(index);
        deathtime = phase - trigger_off_time;
        res_amplitude = (deathtime / _adsr.release_time) * (- get_ADS_amplitude(phase,index)) + get_ADS_amplitude(phase, index);
    }

    if (res_amplitude < MINIMUM_AMPLITUDE) {
        res_amplitude = 0.0f;
    }

    return res_amplitude;
}
