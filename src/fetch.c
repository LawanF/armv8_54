#include <assert.h>
#include "fetch.h"

/*
    A function that takes the address held in the Program Counter
    and returns the Instruction held at that address
*/
uint32_t fetch(MachineState *machine_state) {
    assert(machine_state != NULL);

    // Reads the address held in the Program Counter
    Register pc = machine_state->program_counter;
    uint32_t pc_address = pc.data; // Only takes the lower bits
    uint32_t instruction = readmem32(pc_address);

    // Return the instruction
    return instruction;
}



