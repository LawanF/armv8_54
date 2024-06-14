#ifndef ENCODE_H
#define ENCODE_H

#include <stdint.h>
#include "instructions.h"

uint32_t encode(const Instruction *inst);

#endif
