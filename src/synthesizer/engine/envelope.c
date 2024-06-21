#include <stdbool.h>
#include "../headers/envelope.h"
#include "../headers/format.h"
#include "../headers/keyboard.h"

#define DEFAULT_ATTACK 0.1f
#define DEFAULT_DECAY 0.6f
#define DEFAULT_RELEASE 0.3f
#define DEFAULT_START_AMPLITUDE 1.0f
#define DEFAULT_SUSTAIN_AMPLITUDE 1.0f

#define MINIMUM_AMPLITUDE 0.001f

struct adsr {   
    float attack_time;
    float decay_time;
    float release_time;

    float start_amplitude;
    float sustain_amplitude; 
};

struct adsr _adsr = {DEFAULT_ATTACK, DEFAULT_DECAY, DEFAULT_RELEASE, DEFAULT_START_AMPLITUDE, DEFAULT_SUSTAIN_AMPLITUDE}; 


void attack_set(float value) {
    _adsr.attack_time = value;
}

float get_attack(void) {
    return _adsr.attack_time;
}

void decay_set(float value) {
    _adsr.decay_time = value;
}

float get_decay(void) {
    return _adsr.decay_time;
}

void release_set(float value) {
    _adsr.release_time = value;
}

float get_release(void) {
    return _adsr.release_time;
}

void start_set(float value) {
    _adsr.start_amplitude = value;
}

float get_start(void) {
    return _adsr.start_amplitude;
}

void sustain_set(float value) {
    _adsr.sustain_amplitude = value;
}

float get_sustain(void) {
    return _adsr.sustain_amplitude;
}

float get_ads(float phase, int index) {
    float trigger_on_time = get_trigger_on_time(index);
    float lifetime = (phase - trigger_on_time) / SAMPLE_RATE;
    float res_amplitude;
    if (lifetime < _adsr.attack_time) {
            // ATTACK
            res_amplitude = _adsr.start_amplitude * (lifetime / _adsr.attack_time);
    } else if (lifetime < _adsr.attack_time + _adsr.decay_time) {
            // DECAY    
            res_amplitude = _adsr.start_amplitude - (_adsr.start_amplitude - _adsr.sustain_amplitude) * ((lifetime - _adsr.attack_time) / _adsr.decay_time);
    } else {
            res_amplitude = _adsr.sustain_amplitude;
    }

    return res_amplitude;
}

// phase may loop around D:
float get_amplitude(float phase, int index) {
    NoteState note_on = get_note_on(index);
    float trigger_off_time; // Might not be defined just yet.
    float deathtime;
    float trigger_off_amplitude;
    float res_amplitude;
    if (note_on == ON) {
        // ADS
        res_amplitude = get_ads(phase, index);
    } else if (note_on == OFF) {
        // RELEASE
        trigger_off_time = get_trigger_off_time(index);
        deathtime = (phase - trigger_off_time) / SAMPLE_RATE;
        trigger_off_amplitude = get_ads(trigger_off_time, index);
        res_amplitude = (deathtime / _adsr.release_time) * (- trigger_off_amplitude) + trigger_off_amplitude;
    } else {
        // Note not initialised. 
        res_amplitude = 0.0f;
    }

    if (res_amplitude < MINIMUM_AMPLITUDE) {
        res_amplitude = 0.0f;
    }

    return res_amplitude;
}
