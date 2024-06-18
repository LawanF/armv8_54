#include <stdlib.h>
#include <stdio.h>
#include "symbol_table.c"
#include "emulate_files/instructions.h"

#define MAX_LINE_LEN         50
#define MAX_NUM_INSTRUCTIONS 100
#define SYMTABLE_LOAD_FACTOR 2.0

int main(int argc, char **argv) {
    // Ensure both input and output filenames are provided
    if (argc != 3) {
        fprintf(stderr, "usage: ./assemble [input_file] [output_file]");
        exit(1);
    }
    char *input_filename = argv[1];
    char *output_filename = argv[2];
    static FILE *input_file = NULL;
    static FILE *output_file = NULL;

    // open files
    input_file = fopen(input_filename, "r");
    if (input_file == NULL) {
        fprintf(stderr, "Error: could not open input file for reading: %s\n", input_filename);
        return EXIT_FAILURE;
    }
    output_file = fopen(output_filename, "wb");
    if (output_file == NULL) {
        fprintf(stderr, "Error: could not open output file for writing binary: %s\n", output_filename);
        fclose(input_file);
        return EXIT_FAILURE;
    } else if (strcmp(input_filename, output_filename) == 0) {
        fprintf(stderr, "Error: input filename is identical to output filename");
        fclose(input_file);
        fclose(output_file);
        return EXIT_FAILURE;
    }

    // initialise if there is anything?
    // Loop through each line until EOF, parsing and encoding as needed and writing to output file
    char input_buffer[MAX_LINE_LEN];
    uint32_t cur_pos = 0;

    SymbolTable known_table = symtable_new(/* load_factor = */ SYMTABLE_LOAD_FACTOR);
    SymbolTable unknown_table = symtable_new(/* load_factor = */ SYMTABLE_LOAD_FACTOR);

    Instruction instructions[MAX_NUM_INSTRUCTIONS];
    while ( fgets(input_buffer, MAX_LINE_LEN, input_file) != NULL ) {
        // TODO: resizing array for instructions
        buffer[sizeof(buffer)-1] = NULL;
        src = buffer;
        Instruction inst;
        is_valid = parse_instruction(src, &inst, cur_pos, known_table, unknown_table); 
        if (!(is_valid)) {
            fprintf(stderr, "invalid instruction");
            return 1;
        }
        instructions[cur_pos] = inst;
        cur_pos++;
    }

    for (int i = 0; i < sizeof(instructions); i++) {
        write_binary_instruction(&inst, output_file);
    }

    symtable_free(known_table);
    symtable_free(unknown_table);

    return EXIT_SUCCESS;
}
