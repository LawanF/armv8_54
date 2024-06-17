#include <stdlib.h>
#include "parse_arith.h"
#include "parser.h"

// The maximum number of registers mentioned in an arithmetic instruction:
// 2 for DP (immediate) and 3 for DP (register)
#define MAX_ARITH_IMM_REGS 2
#define IMM12_MAX ((1UL << 12) - 1)
typedef enum { ADD, ADDS, SUB, SUBS } ArithOpc;

/** Parses the immediate form of an arithmetic < op2 > operand, a string of
 * the form "#imm{, lsl #(0|12)}", and fills in the corresponding fields in the
 * `Instruction`, setting the instruction type to DP (immediate).
 * Note that the opcode and register (rn) for this instruction will need to be
 * set separately.
 * @returns true if and only if the operand matches, and parsing succeeds
 */
static bool parse_arith_imm_op2(
    char **src,
    uint8_t rn,
    Instruction *instruction
) {
    Instruction inst = *instruction;
    char *s = *src;
    uint32_t immediate;
    DiscreteShift shift = ZERO_SHIFT;
    bool success = parse_immediate(&s, &immediate)
                && immediate <= IMM12_MAX;
    if (!success) return false;
    // parse optional shift
    parse_discrete_shift(&s, &shift);
    // fill instruction data
    inst.command_format = DP_IMM;
    inst.dp_imm.operand_type = ARITH_OPERAND;
    DPImmOperand *operand = &inst.dp_imm.operand;
    operand->arith_operand.imm12 = immediate;
    operand->arith_operand.rn = rn;
    operand->arith_operand.sh = shift;
    // set instruction and string pointer
    *src = s;
    *instruction = inst;
    return true;
}


/** Parses the register form of an arithmetic < op2 > operand, a
 * string of the form "Rn{, [shift] #[imm]}", and fills in the corresponding
 * Note that the opcode and register (rn) for this instruction will need to be
 * set separately.
 * @returns true if and only if the operand matches, and parsing succeeds.
 */
static bool parse_arith_reg_op2(
    char **src,
    uint8_t rn,
    Instruction *instruction
) {
    Instruction inst = *instruction;
    char *s = *src;
    RegisterWidth rm_width;
    ShiftType shift_type = 0;
    uint8_t shift_amount = 0;
    uint8_t rm;
    bool success = parse_reg(&s, &rm, &rm_width)
                && (rm_width == inst.sf);
    if (!success) return false;
    // parse optional shift
    parse_immediate_shift(&s, &shift_type, &shift_amount);
    inst.command_format = DP_REG;
    // For an arithmetic instruction, (M, opr) if of the form (0, 1xx0)
    // where the xx bits form opr.
    inst.dp_reg.m = 0;
    inst.dp_reg.opr = 4 | (shift_type << 1); // (m, opr) = (0, 1xx0)
    inst.dp_reg.rn = rn;
    inst.dp_reg.rm = rm;
    inst.dp_reg.operand = shift_amount;
    return true;
}

/** Parses an instruction that has one of the forms:
 * [add|adds|sub|subs] Rd, Rn, #imm{, lsl #(0|12)}
 * [cmp|cmn] Rn, #imm{, lsl #(0|12)}
 * [neg|negs] Rd, #imm{, lsl #(0|12)}
 * @returns true and modifies src to point towards the beginning of the
 * unconsumed input, if and only if parsing succeeds
 */
bool parse_add_sub(char **src, Instruction *instruction) {
    Instruction inst = *instruction;
    char *s = *src;
    ArithOpc opc;
    bool success;
    // mnemonics and the aliases they map to
    const char *const add_types[]  = {"add", NULL};
    const char *const adds_types[] = {"adds", "cmn", NULL};
    const char *const sub_types[]  = {"sub", "neg", NULL};
    const char *const subs_types[] = {"cmp", "subs", "negs", NULL};
    // set opcode
    const char *mnemonic;
    // choose this order so that "adds" is checked before "add"
    // in order to select the correct opcode
    if      (parse_from(&s, adds_types, &mnemonic)) { opc = ADDS; }
    else if (parse_from(&s, add_types,  &mnemonic)) { opc = ADD;  }
    else if (parse_from(&s, subs_types, &mnemonic)) { opc = SUBS; }
    else if (parse_from(&s, sub_types,  &mnemonic)) { opc = SUB;  }
    else return false;
    // set registers
    uint8_t rd = ZERO_REG_INDEX;
    uint8_t rn = ZERO_REG_INDEX;
    // the number of registers to parse for DP (immediate):
    // the number for DP (register) will be one greater
    int num_registers = 0;
    uint8_t *register_indices[MAX_ARITH_IMM_REGS];
    // reset string back to original to check how many registers to add
    s = *src;
    if (match_string(&s, "cmp") || match_string(&s, "cmn")) {
        // "cmp Rn, <op2>" is an alias for "subs Rzr, Rn, <op2>"
        // (and the same for cmn with adds)
        num_registers = 1;
        register_indices[0] = &rn;
    } else if (match_string(&s, "negs") || match_string(&s, "neg")) {
        // "neg(s) rd, <op2>" is an alias for "sub(s) rd, rzr, <op2>"
        num_registers = 1;
        register_indices[0] = &rd;
    } else {
        // all other instructions have two operands rd and rn
        num_registers = 2;
        register_indices[0] = &rd;
        register_indices[1] = &rn;
    }
    // reset string back to original and parse the operands
    s = *src;
    RegisterWidth cur_width;
    RegisterWidth next_width;
    success = match_string(&s, mnemonic)
           && skip_whitespace(&s);
    if (!success) return false;
    // check all registers are of the same width
    for (int i = 0; i < num_registers; i++) {
        success = parse_reg(&s, register_indices[i], &cur_width)
               && match_char(&s, ',')
               && skip_whitespace(&s);
        if (!success) return false;
        if (i == 0) {
            next_width = cur_width;
        } else if (cur_width != next_width) return false;
    }
    // set this width to be the overall width of the instruction
    inst.sf = cur_width;
    // now parse < op2 >
    // first attempt to interpret it as a DP (immediate) instruction
    success = parse_arith_imm_op2(&s, rn, &inst)
           || parse_arith_reg_op2(&s, rn, &inst);
    if (!success) return false;
    // set rd and opc
    inst.rd = rd;
    inst.opc = opc;
    *src = s;
    *instruction = inst;
    return true;
}
