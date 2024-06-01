#include "registers.h"
#include <assert.h>
#include <stdio.h>

static MachineState machine_state;

void init_machine_state(void) {
    // Creating a pointer to the machine state to alter the global var
    MachineState *ms = &machine_state;

    // Initialising the general registers
    for (int i = 0; i < NUM_GENERAL_REGISTERS; i++) {
        ms->general_registers[i].data = 0;
        ms->general_registers[i].writable = 1;
    }

    // Initialising the special registers
    ms->zero_register.data = 0;
    ms->zero_register.writable = 0;

    ms->program_counter.data = 0;
    ms->program_counter.writable = 1;

    ms->pstate.zero = 0;
    ms->pstate.neg = 0;
    ms->pstate.carry = 0;
    ms->pstate.overflow = 0;
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
    assert(index <= NUM_GENERAL_REGISTERS);
    assert(index >= 0);

    // Find the general register and write
    MachineState *ms = &machine_state;
    ms->general_registers[index].data = value; 
}

