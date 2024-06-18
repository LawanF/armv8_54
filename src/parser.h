#ifndef PARSER_H
#define PARSER_H

#include <stdint.h>
#include <stdlib.h>
#include "symbol_table.h"
#include "emulate_files/instructions.h"

#define NUM_DP_IMM_INSTS 22
#define NUM_DP_REG_INSTS 4
#define NUM_BRANCH_INSTS 9
#define NUM_SDT_INSTS 2

// the representation of the zero register
#define ZERO_REG_INDEX 31

#define ARRAY_LEN(arr) (sizeof(arr) / sizeof(void *))

extern const char *const branch_conds[];
extern const char *const shift_types[];

typedef enum { COND, UNCOND, LOAD } LiteralInstr;
typedef enum { LSL, LSR, ASR, ROR } ShiftType;

void set_offset(Instruction *inst, uint32_t cur_pos, uint32_t target_pos);
bool skip_whitespace(char **src);
bool parse_label(char **src, uint32_t inst_pos, SymbolTable table);
bool parse_directive(char **src, int32_t *dest);
bool parse_instruction(char **src, Instruction *instruction, uint32_t cur_pos, SymbolTable known_table, SymbolTable unknown_table);

#endif
