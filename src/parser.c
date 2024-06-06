#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "parser.h"
#include "instructions.h"
#define binsearch(list, list_size) {(char *) bsearch(type_ptr, list[0], list_size, sizeof(char *), strcmp)}


void set_command_format(char *type, Instruction *current_instr) {
    char **type_ptr = &type;
    char *found_type;

    found_type = binsearch(str_dp_imm, NUM_DP_IMM_INSTS);
    if (found_type != NULL) { current_instr->command_format = DP_IMM; }

    found_type = binsearch(str_dp_reg, NUM_DP_REG_INSTS);
    if (found_type != NULL) { current_instr->command_format = DP_REG; }

    found_type = binsearch(str_branch, NUM_BRANCH_INSTS);
    if (found_type != NULL) { current_instr->command_format = BRANCH; }

    found_type = binsearch(str_sdt, NUM_SDT_INSTS);
    if (found_type != NULL) { /* uhhh what do i do here idk bc like it could be 
                             load literal or single data transfer */ }

    current_instr->command_format = UNKNOWN;
}

bool parse_register(char *reg_str) {
    
}


Instruction *decode_string(char *line, char **line_ptr) {
    Instruction instruction;

    // Set the command format
}



