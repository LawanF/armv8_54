#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "headers/execute.h"
#include "headers/decode.h"
#include "headers/fetch.h"
#include "headers/fileio.h"
#include "headers/memory.h"
#include "headers/registers.h"

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

/*
    Runs the emulator, taking the command line arguments.
    Returns 0 upon successful termination.
*/
int run_emulator(int argc, char **argv) {
    // Check number of arguments.
    if (argc > 3 || argc == 1) {
        fprintf(stderr, "usage: ./emulate [input_file] [optional_output_file]");
        exit(1);
    }

    // If output file is given, store it in output_file.
    if (argc == 3) {
        output_file = argv[2];
    }

    // Initialise machine state and memory, and load the input file.
    initialise();
    store_file_to_mem(argv[1]);

    // Run the machine, waiting for the halt instruction to exit.
    while (1) {
        MachineState machine_state = read_machine_state();
        uint32_t inst_data = fetch(&machine_state);
        Instruction inst = decode(inst_data);
        execute(&inst);
        increment_pc();
    }

    return 0;
}

int main(int argc, char **argv) {
    return run_emulator(argc, argv);
}
