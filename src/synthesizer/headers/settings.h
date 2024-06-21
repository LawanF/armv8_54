#ifndef SETTINGS_H
#define SETTINGS_H

#define VOLUME_MAX 1.0f
#define ATTACK_MAX 20.0f
#define DECAY_MAX 20.0f
#define SUSTAIN_MAX 1.0f
#define RELEASE_MAX 20.0f

#define VOLUME_STEP 0.01f
#define ATTACK_STEP 0.01f
#define DECAY_STEP 0.01f
#define SUSTAIN_STEP 0.01f
#define RELEASE_STEP 0.01f

void setting_right(void);

void setting_left(void);

void setting_adjust(bool up);

#endif
