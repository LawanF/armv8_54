#include <stdbool.h>
#include <stdlib.h>
#include "../headers/envelope.h"
#include "../headers/settings.h"
#include "../headers/waveforms.h"

#define NUMBER_OF_SETTINGS 5

typedef void (*setting_function)(float);

typedef enum {VOLUME, ATTACK, DECAY, SUSTAIN, RELEASE} SETTING_TYPE;

static SETTING_TYPE current_setting = VOLUME;

void setting_right(void) {
    if (current_setting < NUMBER_OF_SETTINGS - 1) {
        current_setting++;
    }
}

void setting_left(void) {
    if (current_setting > 0) {
        current_setting--;
    }
}

void setting_adjust(bool up) {
    int adjust_factor = up ? 1 : -1;
    char *setting_string;
    float setting_value;
    float setting_step;
    float setting_max;
    setting_function func;
    switch (current_setting) {
        case VOLUME:
            setting_string = "Volume";
            setting_value = get_volume();
            setting_step = VOLUME_STEP;
            setting_max = VOLUME_MAX;
            func = &volume_set;
            break;
        case ATTACK:
            setting_string = "Attack";
            setting_value = get_attack();
            setting_step = ATTACK_STEP;
            setting_max = ATTACK_MAX;
            func = &attack_set;
            break;
        case DECAY:
            setting_string = "Decay";
            setting_value = get_decay();
            setting_step = DECAY_STEP;
            setting_max = DECAY_MAX;
            func = &decay_set;
            break;
        case SUSTAIN:
            setting_string = "Sustain";
            setting_value = get_sustain();
            setting_step = SUSTAIN_STEP;
            setting_max = SUSTAIN_MAX;
            func = &sustain_set;
            break;
        case RELEASE:
            setting_string = "Release";
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