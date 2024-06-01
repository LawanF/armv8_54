#ifndef DECODE_H
#define DECODE_H

#include <stdint.h>
#include "instructions.h"

Instruction decode(uint32_t inst_data);

#endif
