#include <stdint.h>
#include <stdbool.h>

typedef struct {
    uint64_t data;
} GeneralRegister;

typedef struct {
    bool zero;
    bool neg;
    bool carry;
    bool overflow;
} ProcessorStateRegister;

