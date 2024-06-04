#include <stdlib.h>
#include <string.h>
#include "execute.h"
#include "decode.h"
#include "fetch.h"
#include "fileio.h"
#include "memory.h"
#include "registers.h"

// Pointer to output file name if it is given.
static char *output_file = NULL;

/* 
    Function to return the output file name pointer.
*/
char *get_output_file(void) { return output_file; }

/*
    Function to initialise the machine
*/
static void initialise(void) {
    initmem();
    init_machine_state();
}

int main(int argc, char **argv) {
    // Check number of arguments.
    if (argc > 3 || argc == 1) {
        fprintf(stderr, "usage: ./emulate [input_file] [optional_output_file]");
        exit(1);
    }

    // If output file is given, store it in output_file.
    if (argc == 3) {
        strcpy(output_file, argv[2]);
    }

    // Initialise machine state and memory, and load the input file.
    initialise();
    store_file_to_mem(argv[1]);

    // Run the machine, waiting for the halt instruction to exit.
    while (1) {
        MachineState machine_state = read_machine_state();
        uint64_t cur_pc = machine_state.program_counter.data;
        execute(decode(cur_pc));
        increment_pc();
    }

    return 0;
}
