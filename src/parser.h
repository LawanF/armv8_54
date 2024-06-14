#ifndef PARSER_H
#define PARSER_H

#include <stdint.h>

#define NUM_DP_IMM_INSTS 22
#define NUM_DP_REG_INSTS 4
#define NUM_BRANCH_INSTS 9
#define NUM_SDT_INSTS 2

#define ARRAY_LEN(arr) (sizeof(arr) / sizeof(void *))

const char *const arith_ops[]     = {"add", "adds", "sub", "subs"};
const char *const logic_ops[]     = {"and", "ands", "bic", "bics", "eon", "eor", "orn", "orr"};
const char *const comp_ops[]      = {"cmn", "cmp"};
const char *const neg_ops[]       = {"neg", "negs"};
const char *const w_move_ops[]    = {"movk", "movn", "movz"};
const char *const mul_arith_ops[] = {"madd", "msub"};
const char *const mul_neg_ops[]   = {"mneg", "mul"};

typedef enum regwidth { _32_BIT, _64_BIT } RegisterWidth;
typedef enum { LSL, LSR, ASR, ROR } ShiftType;
typedef enum { ZERO_SHIFT, TWELVE_SHIFT } DiscreteShift;
const char *const shift_types[]   = {"lsl", "lsr", "asr", "ror"};

bool match_char(char **src, const char token);
bool match_string(char **src, const char *token);
bool parse_from(char **src, char **tokens, char **chosen);
bool skip_whitespace(char **src);
bool parse_uint(char **src, uint32_t *dest, int base);
bool parse_int(char **src, int32_t *dest, int base);
bool parse_immediate(char **src, uint32_t *dest);
bool parse_reg(char **src, int *index, RegisterWidth *width);
bool parse_discrete_shift(char **src, DiscreteShift *shift);
bool parse_immediate_shift(char **src, ShiftType *shift_type, uint8_t *shift_amount);

#endif
