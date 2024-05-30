#include "registers.h"

static MachineState machine_state;

void init_machine_state(void) {
    // Initialising the general registers
    Register reg_arr[NUM_GENERAL_REGISTERS];
    for (int i = 0; i < NUM_GENERAL_REGISTERS; i++) {
        Register current_reg;
        current_reg.writable = 1;
        current_reg.data = 0;
        reg_arr[i] = current_reg;
    }

    // Initialising the zero register
    Register zero;
    zero.writable = 0;
    zero.data = 0;

    // Initialising the program counter
    Register pc;
    pc.writable = 1;
    pc.data = 0;

    // Initialising the processor state register
    ProcessorStateRegister pstate_reg;
    pstate_reg.zero = 0;
    pstate_reg.neg = 0;
    pstate_reg.carry = 0;
    pstate_reg.overflow = 0;

    // Putting all the registers into the machine state
    machine_state.general_registers = reg_arr;
    machine_state.zero_register = zero;
    machine_state.program_counter = pc;
    machine_state.pstate = pstate_reg;
}

void 
