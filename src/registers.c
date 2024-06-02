#include "registers.h"
#include "memory.h"
#include <assert.h>

static MachineState machine_state;

void init_machine_state(void) {
    // Creating a pointer to the machine state to alter the global var
    MachineState *ms_pointer = &machine_state;

    // Initialising the general registers
    for (int i = 0; i < NUM_GENERAL_REGISTERS; i++) {
        ms_pointer->general_registers[i].data = 0;
        ms_pointer->general_registers[i].writable = 1;
    }

    // Initialising the special registers
    ms_pointer->zero_register.data = 0;
    ms_pointer->zero_register.writable = 0;

    ms_pointer->program_counter.data = 0;
    ms_pointer->program_counter.writable = 1;

    ms_pointer->pstate.zero = 0;
    ms_pointer->pstate.neg = 0;
    ms_pointer->pstate.carry = 0;
    ms_pointer->pstate.overflow = 0;
}

/*
    A function that returns a copy of the machine_state (meant to be
    read-only)
*/
MachineState read_machine_state(void) {
    return machine_state;
}


/*
    A function that writes to a specific general register given
    the required index, and the value that needs to be written
*/
void write_general_registers(int index, uint64_t value) {
    // Check that the index refers to an existing general register
    assert(index < NUM_GENERAL_REGISTERS);
    assert(index >= 0);

    // Find the general register and write
    MachineState *ms_pointer = &machine_state;
    ms_pointer->general_registers[index].data = value;
}

/*
    A function that writes to the program counter a specific value,
    the logic of what that might be is  dealt with separately
*/
void write_program_counter(uint32_t address) {
    checkaddress32(address);
    MachineState *ms_pointer = &machine_state;
    ms_pointer->program_counter.data = address;
}

void increment_pc(void) {
    MachineState *ms_pointer = &machine_state;
    write_program_counter((ms_pointer->program_counter.data) + 4);
}

/*
    A function that sets pstate flags in the machine state, given
    the flag they want to alter
*/
void set_pstate_flag(char flag, bool value) {
    assert((flag == 'N') || (flag == 'C') || (flag == 'V') || (flag == 'Z'));
    MachineState *ms_pointer = &machine_state;
    switch (flag) {
        case 'N':
            ms_pointer->pstate.neg = value;
            break;
        case 'C':
            ms_pointer->pstate.carry = value;
            break;
        case 'V':
            ms_pointer->pstate.overflow = value;
            break;
        case 'Z':
            ms_pointer->pstate.zero = value;
            break;
        default:
            break;
    }
}
