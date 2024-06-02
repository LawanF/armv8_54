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

extern void init_machine_state(void)

extern MachineState read_machine_state(void)

extern void write_general_registers(int index, uint64_t value)

extern void write_program_counter(uint32_t address)

extern void increment_pc(void)

extern void set_pstate_flag(char flag, bool value)

#endif
