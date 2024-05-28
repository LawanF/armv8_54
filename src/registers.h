#ifndef REGISTERS_H
#define REGISTERS_H

#include <stdint.h>
#include <stdbool.h>

#define NUM_GENERAL_REGISTERS 31

typedef struct {
    // Denotes whether the register can be written to by an instruction.
    bool writable;
    uint64_t data;
} Register;

typedef struct {
    bool zero    :1;
    bool neg     :1;
    bool carry   :1;
    bool overflow:1;
} ProcessorStateRegister;

typedef struct {
    Register general_registers[NUM_GENERAL_REGISTERS];
    Register zero_register;
    Register program_counter;
    ProcessorStateRegister pstate;
} MachineState;

#endif
