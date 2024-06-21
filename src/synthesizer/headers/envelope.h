typedef struct adsr *ADSR;

void attack_set(float value);

float get_attack(void);

void decay_set(float value);

float get_decay(void);

void release_set(float value);

float get_release(void);

void start_set(float value);

float get_start(void);

void sustain_set(float value);

float get_sustain(void);

float get_amplitude(float phase, int index);