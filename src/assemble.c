#include <stdlib.h>

static char *input_file = NULL;
static char *output_file = NULL;

int main(int argc, char **argv) {

    if (!(argc == 3)) {
        fprintf(stderr, "usage: ./assemble [input_file] [output_file]");
        exit(1);
    }

    output_file = argv[2];
    input_file = argv[1];

    // initialise if there is anything?

    // loop read each line until EOF. parse each line, pass to encode, pass to binary file writer.
    // where does two pass come in
    char buffer[20];
    uint32_t cur_pos = 0;

    SymbolTable known_table = symtable_new(2);
    SymbolTable unknown_table = symtable_new(2);

    Instruction instructions[100];
    char **src;
    bool is_valid;
    while ( fgets(buffer, sizeof(buffer), in) != NULL ) {
        // ADD: resizing array for instructions
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
