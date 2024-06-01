#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "registers.h"
#include "memory.h"
#define WORD_SIZE (4)

/*
    Takes a FILE *fileptr and a long int *size.
    Stores the size of file (in bytes) in size.
*/
static void filesize(FILE *fileptr, long int *size) {
    if (fseek(fileptr, 0, SEEK_END)) {
        fprintf(stderr, "filesize: fseek error, errno %d\n", errno);
        exit(1);
    }

    if ((*size = ftell(fileptr)) < 0) {
        fprintf(stderr, "filesize: ftell error, errno %d\n", errno);
        exit(1);
    }

    if (fseek(fileptr, 0, SEEK_SET)) {
        fprintf(stderr, "filesize: fseek error, errno  %d\n", errno);
        exit(1);
    }
}

/*
    Takes a string that specifies a file location.
    Returns a char array with the contents of that file.
*/
char *store_file_to_arr(char *filename) {
    // Define array to return.
    char *arr;

    // Open file and check if it opens successfully.
    FILE *fileptr = fopen(filename, "r");
    if (fileptr == NULL) {
        fprintf(stderr, "store_file_to_arr: can’t open %s, errno %d\n", filename, errno);
        exit(1);
    }

    // Store filesize in size.
    long int size;
    filesize(fileptr, &size);

    // Allocate memory to arr and check if allocation is successful.
    arr = malloc(size);
    if (arr == NULL) {
        fprintf(stderr, "store_file_to_arr: ran out of memory, errno %d\n", errno);
        exit(1);
    }

    // Read file into arr.
    fread(arr, 1, size, fileptr);
    if (ferror(fileptr)) {
        fprintf(stderr, "store_file_to_arr: can't read %s, errno %d\n", filename, errno);
    }

    // Close file.
    fclose(fileptr);

    return arr;
}

// OUTPUT FILE
/*
    A function that executes printf but flags errors (to help with
    file writing errors)
*/
static void printf_with_err(char *output, va_list args) {
    int ret = printf(output, args);
    if (ret < 0) {
	fprintf(stderr, "printf_with_err: couldn't print instruction of format %s", output);
    }
}

/*
    A function that locates memory locations with non-zero data
    and prints them for the output file
*/
static void locate_non_zero_mem(void) {
    for (int i = 0; i < MEMORY_SIZE; i += WORD_SIZE) {
        uint32_t data = readmem32(i);
        if (data != 0) {
            printf_with_err("%08x: %08x\n", i, data);
        }
    }
}

/*
    A function that puts the final outputs of the registers, condition
    flags and memory into stdout
*/
void print_output(MachineState *machine_state, char *filename) {
    // Write to the file if it exists
    FILE *output;
    if (filename != NULL) {
	output = freopen(filename, "a+", stdout);
	if (output == NULL) {
	    fprintf(stderr, "print_output: can't open %s, errno %d\n", filename, errno);
	}
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
    locate_non_zero_mem();

    if (filename != NULL) {
	fclose(output);
    }
}
