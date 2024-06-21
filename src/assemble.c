#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "headers/symbol_table.h"
#include "headers/parser.h"
#include "headers/encode.h"
#include "headers/instructions.h"
#include "headers/assemble.h"

#define FREE_TABLES() symtable_free(known_table); symtable_free(unknown_table);
#define FAIL_RUNNING_PROGRAM() fclose(input_file); \
    fclose(output_file); return EXIT_FAILURE;

#define MAX_LINE_LEN         50
#define MAX_NUM_INSTRUCTIONS 100
#define SYMTABLE_LOAD_FACTOR 2.0

int run_assembler(int argc, char **argv) {
    // Ensure both input and output filenames are provided
    if (argc != 3) {
        fprintf(stderr, "usage: ./assemble [input_file] [output_file]\n");
        return EXIT_FAILURE;
    }
    char *input_filename = argv[1];
    char *output_filename = argv[2];
    static FILE *input_file = NULL;
    static FILE *output_file = NULL;

    // Open input and output files
    input_file = fopen(input_filename, "r");
    if (input_file == NULL) {
        fprintf(stderr, "Error: could not open input file for reading: %s\n", input_filename);
        return EXIT_FAILURE;
    }
    output_file = fopen(output_filename, "w");
    if (output_file == NULL) {
        fprintf(stderr, "Error: could not open output file for writing binary: %s\n", output_filename);
        fclose(input_file);
        return EXIT_FAILURE;
    } else if (strcmp(input_filename, output_filename) == 0) {
        fprintf(stderr, "Error: input filename is identical to output filename");
        FAIL_RUNNING_PROGRAM();
    }

    // Loop through each line until EOF, parsing and encoding as needed and writing to output file
    char input_buffer[MAX_LINE_LEN];
    uint32_t cur_pos = 0;

    SymbolTable known_table = symtable_new(/* load_factor = */ SYMTABLE_LOAD_FACTOR);
    if (known_table == NULL) {
        fprintf(stderr, "Error: failed to create known symbol table\n");
        FAIL_RUNNING_PROGRAM();
    }
    SymbolTable unknown_table = symtable_new(/* load_factor = */ SYMTABLE_LOAD_FACTOR);
    if (unknown_table == NULL) {
        fprintf(stderr, "Error: failed to create unknown symbol table\n");
        FAIL_RUNNING_PROGRAM();
    }

    ProgramLine program[MAX_NUM_INSTRUCTIONS];
    while ( cur_pos < MAX_NUM_INSTRUCTIONS && fgets(input_buffer, MAX_LINE_LEN, input_file) != NULL ) {
        // TODO: resizing array for instructions
        // replace newline if it exists
        char *newline = NULL;
        if ((newline = strchr(input_buffer, '\n')) != NULL) {
            *newline = '\0';
        }
        Instruction inst;
        int32_t directive;
        char *unconsumed = input_buffer;
        // skip any indent
        skip_whitespace(&unconsumed);
        ProgramLine *cur_line = &program[cur_pos];
        if (parse_instruction(&unconsumed, &inst, cur_pos, known_table, unknown_table)) {
            // Write instruction to buffer
            cur_line->is_instruction = true;
            cur_line->data.inst = inst;
            cur_pos++;
        }
        else if (parse_directive(&unconsumed, &directive)) {
            // Write directive to buffer
            cur_line->is_instruction = false;
            cur_line->data.directive = directive;
            cur_pos++;
        } else if (parse_label(&unconsumed, cur_pos, known_table)) {
            // Correct all forward references from unknown table
            uint32_t back_line;
            char *label = unconsumed;
            while (multi_symtable_remove_last(unknown_table, label, &back_line)) {
                set_offset(&program[back_line].data.inst, /* inst_pos = */ back_line, /* target_pos = */ cur_pos);
            }
            multi_symtable_remove_all(unknown_table, label, NULL);
        } else if (!(skip_whitespace(&unconsumed) || *unconsumed == '\0')){
            // unknown, non-empty input
            fprintf(stderr, "Error: unknown input %s\n", unconsumed);
            // free any existing structures
            FREE_TABLES();
            FAIL_RUNNING_PROGRAM();
        }
    }

    if (!symtable_empty(unknown_table)) {
        // some forward references still unknown
        fprintf(stderr, "Error: unknown forward references in input file.\n");
        FREE_TABLES();
        FAIL_RUNNING_PROGRAM();
    }

    for (int pos = 0; pos < cur_pos; pos++) {
        ProgramLine cur_line = program[pos];
        uint32_t encoded = cur_line.is_instruction
            ? encode(&cur_line.data.inst)
            : (uint32_t) cur_line.data.directive;
        // write encoded file byte by byte
        for (int i = 0; i < sizeof(uint32_t); i++) {
            char b = (char) (encoded & 0xFF);
            if (putc(b, output_file) == EOF) {
                // error when writing to output file
                fprintf(stderr, "Error: writing line %d to output file failed\n", i);
                FREE_TABLES();
                FAIL_RUNNING_PROGRAM();
            }
            encoded >>= 8;
        }
    }

    FREE_TABLES();
    fclose(input_file);
    fclose(output_file);

    return EXIT_SUCCESS;
}

int main(int argc, char **argv) {
    return run_assembler(argc, argv);
}
