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
