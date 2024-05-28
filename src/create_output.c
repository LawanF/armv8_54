#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <stdarg.h>
#include "registers.h"
#include "memory.h"

/* A function that puts the final outputs of the registers, condition flags 
and memory into stdout */

void print_output(MachineState *machine_state, char *filename) {
	// Write to the file if it exists
	FILE *output;
	if (filename != NULL) {
		output = freopen(filename, "a+", stdout);
		if (output == NULL) {
			fprintf(stderr, "print_output: can't open %s, errno %d\n", filename, errno);
	}

	// Printing register content
	printf_with_err("Registers:\n");

	// Prints the output of each general register
	for (int i = 0; i < NUM_GENERAL_REGISTERS; i++) {
		Register gen_reg = machine_state->general_registers[i];
		printf_with_err("X%02d = %016x\n", i, gen_reg.data);
	}

	// Prints the output of the program counter
	Register pc = machine_state->program_counter;
	printf_with_err("PC = %016x\n", pc.data);

	// Prints condition flags in PSTATE
	ProcessorStateRegister pstate = machine_state->pstate;
	printf_with_err("PSTATE : %c%c%c%c\n", pstate.neg ? 'N' : '-', pstate.zero ? 'Z' : '-', pstate.carry ? 'C' : '-', pstate.overflow ? 'V' : '-');

	// Printing non-zero memory
	printf_with_err("Non-zero memory:");

	// Locates non-zero memory and prints the data
	locate_non-zero_mem();

	if (filename != NULL) {
		fclose(output);
	}
}

void locate_non-zero_mem(void) {
	int mem_size = MEMORY_SIZE / 4;
	for (int i = 4; i < mem_size; i += 4) {
		uint32_t data = readmem32(i);
		if (data != 0) {
			printf_with_err("%08x: %08x\n", i, data);
		}
	}
}

void printf_with_err(char *output, va_list args) {
	int ret = printf(output, args);
    	if (ret < 0) {
        	fprintf(stderr, "printf_with_err: couldn't print instruction of format %s", output);
    	}
}
