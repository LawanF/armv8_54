#include <stdlib.h>
#include "registers.h"
#include "memory.h"

/* A function that puts the final outputs of the registers, condition flags 
and memory into stdout */

void print_output(MachineState *machine_state) {
	// Printing register content
	printf("Registers:\n");

	// Prints the output of each general register
	for (int i = 0; i < NUM_GENERAL_REGISTERS; i++) {
		Register gen_reg = machine_state->general_registers[i];
		printf("X%02d = %016x\n", i, gen_reg.data);
	}

	// Prints the output of the program counter
	Register pc = machine_state->program_counter;
	printf("PC = %016x\n", pc.data);

	// Prints condition flags in PSTATE
	ProcessorStateRegister pstate = machine_state->pstate;
	printf("PSTATE : %c%c%c%c\n", pstate.neg ? 'N' : '-', pstate.zero ? 'Z' : '-', pstate.carry ? 'C' : '-', pstate.overflow ? 'V' : '-');

	// Printing non-zero memory
	printf("Non-zero memory:");

	// Locates non-zero memory
}

void locate-non-zero_mem() {
	for (
}

