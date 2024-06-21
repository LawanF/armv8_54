#include <stdbool.h>
#include <stdlib.h>
#include "../headers/envelope.h"
#include "../headers/settings.h"
#include "../headers/waveforms.h"

#define NUMBER_OF_SETTINGS (RELEASE + 1)

typedef void (*setting_function)(float);

typedef enum {VOLUME, OSCILLATOR, OCTAVE, ATTACK, DECAY, SUSTAIN, RELEASE} SETTING_TYPE;

static SETTING_TYPE current_setting = VOLUME;

static char *fetch_setting_string(SETTING_TYPE setting_type) {
    switch (setting_type) {
        case VOLUME:
            return "Volume";
        case OSCILLATOR:
            return "Oscillator";
        case OCTAVE:
            return "Octave";
        case ATTACK:
            return "Attack";
        case DECAY:
            return "Decay";
        case SUSTAIN:
            return "Sustain";
        case RELEASE:
            return "Release";
    }
}

void setting_right(void) {
    if (current_setting < NUMBER_OF_SETTINGS - 1) {
        current_setting++;
    }
    printf("Setting is %s.\n", fetch_setting_string(current_setting));
}

void setting_left(void) {
    if (current_setting > 0) {
        current_setting--;
    }
    printf("Setting is %s.\n", fetch_setting_string(current_setting));
}

void setting_adjust(bool up) {
    int adjust_factor = up ? 1 : -1;
    char *setting_string = fetch_setting_string(current_setting);
    float setting_value;
    float setting_step;
    float setting_max;
    setting_function func;
    switch (current_setting) {
        case VOLUME:
            setting_value = get_volume();
            setting_step = VOLUME_STEP;
            setting_max = VOLUME_MAX;
            func = &volume_set;
            break;
        case OSCILLATOR: // Exception case.
            oscillator_adjust(up);
            return;
        case OCTAVE: // Exception case.
            octave_adjust(up);
            return;
        case ATTACK:
            setting_value = get_attack();
            setting_step = ATTACK_STEP;
            setting_max = ATTACK_MAX;
            func = &attack_set;
            break;
        case DECAY:
            setting_value = get_decay();
            setting_step = DECAY_STEP;
            setting_max = DECAY_MAX;
            func = &decay_set;
            break;
        case SUSTAIN:
            setting_value = get_sustain();
            setting_step = SUSTAIN_STEP;
            setting_max = SUSTAIN_MAX;
            func = &sustain_set;
            break;
        case RELEASE:
            setting_value = get_release();
            setting_step = RELEASE_STEP;
            setting_max = RELEASE_MAX;
            func = &release_set;
            break;
    }

    float adjusted_step = adjust_factor * setting_step;
    float adjusted_value = setting_value + adjusted_step;

    if (adjusted_value <= 0.0f) {
        adjusted_value = 0.0f;
        printf("%s is MIN!\n", setting_string);
    } else if (adjusted_value >= setting_max) {
        adjusted_value = setting_max;
        printf("%s is MAX!\n", setting_string);
    }
    
    (*func)(adjusted_value);
    printf("%s: %f\n", setting_string, adjusted_value);
}