#ifndef PARSER_H
#define PARSER_H
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

typedef enum { LSL, LSR, ASR, ROR } shift_type;
typedef enum { ZERO_SHIFT, TWELVE_SHIFT } discrete_shift;
const char *const shift_types[]   = {"lsl", "lsr", "asr", "ror"};

#endif
