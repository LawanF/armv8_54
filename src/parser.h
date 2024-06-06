#ifndef PARSER_H
#define PARSER_H
#define NUM_DP_IMM_INSTS 22
#define NUM_DP_REG_INSTS 4
#define NUM_BRANCH_INSTS 9
#define NUM_SDT_INSTS 2

char *str_dp_imm[] = {"add", "adds", "and", "ands", "bic", "bics", "cmn", "cmp", "eon", "eor", "mov", "movk", "movn", "movz", "mvn", "neg", "negs", "orn", "orr", "sub", "subs", "tst"};
char *str_dp_reg[] = {"madd", "mneg", "msub", "mul"};
char *str_branch[] = {"b", "b.al", "b.eq", "b.ge", "b.gt", "b.le", "b.lt", "b.ne", "br"};
char *str_sdt[] = {"ldr", "str"};

#endif
