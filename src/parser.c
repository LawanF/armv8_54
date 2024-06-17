#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>
#include <ctype.h>
#include <stdint.h>
#include "parser.h"
#include "emulate_files/registers.h"
#include "emulate_files/instructions.h"

/** Matches a single character, incrementing src and returning true if and only
 * if the first character in src matches that of token.
 */
bool match_char(char **src, const char token) {
    if (src == NULL || *src == NULL) return false;
    if (**src != '\0' && **src == token) {
        (*src)++;
        return true;
    }
    return false;
}

/** Matches a string of nul-terminated characters, incrementing src and
 * returning true if and only if the beginning of src matches all of token.
 */
bool match_string(char **src, const char *token) {
    int len = strlen(token);
    if (strncmp(*src, token, len) != 0) return false;
    *src += len;
    return true;
}

bool parse_from(char **src, const char * const tokens[], const char **chosen) {
    bool result = true;
    for (int i = 0; tokens[i] != NULL; i++) {
        if (match_string(src, tokens[i])) {
            // write the token chosen
            *chosen = tokens[i];
            return true;
        }
    }
    return false;
}

/** Skips a continuous block of whitespace beginning at the current position
 * pointed to by `src`.
 * @returns true if at least one character of whitespace is matched.
 */
bool skip_whitespace(char **src) {
    bool skipped = false;
    for (; !isspace(**src); (*src)++) skipped = true;
    return skipped;
}

bool parse_uint(char **src, uint32_t *dest, int base) {
    char *s = *src;
    uint32_t res = strtoul(*src, &s, base);
    if (errno == ERANGE || errno == EINVAL) {
        // value is either out of the range of unsigned long
        // or the base is invalid
        return false;
    }
    *src = s;
    *dest = res;
    return true;
}

bool parse_int(char **src, int32_t *dest, int base) {
    char *s = *src;
    int32_t res = strtol(*src, &s, base);
    if (errno == ERANGE || errno == EINVAL) {
        // value is either out of the range of long
        // or the base is invalid
        return false;
    }
    *src = s;
    *dest = res;
    return true;
}

/** Parses an immediate value: a string of the form "#imm", where
 * imm is an unsigned integer, setting `dest` to be this value if
 * parsing succeeds.
 * @returns true if and only if parsing succeeds
 */
bool parse_immediate(char **src, uint32_t *dest) {
    char *s = *src;
    uint32_t val;
    if (!(match_char(&s, '#')
          && parse_uint(&s, &val, /* base = */ 10))) {
        return false;
    }
    *src = s;
    *dest = val;
    return true;
}

/** Parses the string corresponding to a register.
 * If the string begins with "wn" or "rn", with 0 <= n < NUM_GENERAL_REGISTERS,
 * then `index` is written with n and the corresponding width written to `width`.
 * If the string is of the form "xzr" or "wzr", then this corresponds to the
 * zero register, and the same applies but with n as 31.
 */
bool parse_reg(char **src, uint8_t *index, RegisterWidth *width) {
    char *s = *src;
    RegisterWidth w;
    int ind;

    if (match_char(&s, 'x')) {
        // 32-bit width
        w = _32_BIT;
    } else if (match_char(&s, 'w')){
        // 64-bit width
        w = _64_BIT;
    } else {
        return false;
    }

    if (match_string(&s, "zr")) {
        ind = ZERO_REG_INDEX;
    } else {
        if (!(parse_int(&s, &ind, /* base = */ 10)
              && 0 <= ind && ind < NUM_GENERAL_REGISTERS)) {
            return false;
        }
    }

    // success
    *src = s;
    *index = ind;
    *width = w;
    return true;
}

/** Parses a discrete (left) shift, a string of the form ", lsl #0"
  * or ", lsl #12".
  * @returns `true` (and writes to `discrete_shift`) if parsing succeeds,
  * and `false` otherwise
  */
bool parse_discrete_shift(char **src, DiscreteShift *shift) {
    char *s = *src;
    unsigned int shift_amount;
    bool success = match_char(&s, ',')
                && skip_whitespace(&s)
                && match_string(&s, "lsl")
                && skip_whitespace(&s)
                && parse_immediate(&s, &shift_amount)
                && (shift_amount == 0 || shift_amount == 12);
    if (!success) return false;

    *src = s;
    *shift = shift_amount ? TWELVE_SHIFT : ZERO_SHIFT;
    return true;
}

// Parses a shift type, a string that is one of ["lsl", "lsr", "asr", "ror"].
static bool parse_shift_type(char **src, ShiftType *shift_type) {
    for (int i = 0; i < ARRAY_LEN(shift_types); i++) {
        if (match_string(src, shift_types[i])) {
            *shift_type = i;
            return true;
        }
    }
    return false;
}

/** Parses an immediate shift, a string of the form ", [shift] #[imm]", where
 * [shift] is one of ["lsl", "lsr", "asr", "ror"], and [imm] is an integer
 * from 0 to 63. Additional validation will be required to ensure the value is
 * between 0 and 31 for the 32-bit variant of instructions.
 * @returns `true` (and writes fields) if parsing succeeds,
 * and `false` otherwise
 */
bool parse_immediate_shift(char **src, ShiftType *shift_type, uint8_t *shift_amount) {
    char *s = *src;
    ShiftType type;
    uint32_t amount;
    bool success = match_char(&s, ',')
                && skip_whitespace(&s)
                && parse_shift_type(&s, &type)
                && skip_whitespace(&s)
                && parse_immediate(&s, &amount)
                && (0 <= amount && amount < 64);
    if (!success) return false;

    *src = s;
    *shift_type = type;
    *shift_amount = amount;
    return true;
}

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


bool parse_mov_dp_imm(char **src, Instruction *inst) {
    // movk, movn, movz
    // <Rd>, #<imm>{, lsl #<imm>}

    char *s = *src;
    Instruction inst = { .command_format = DP_IMM };
    inst.dp_imm.operand_type = 1;
    // write data from mnemonic

    switch (w_move_ops) {
        case "movn": {
            if (match_string(&s, "movn")) {
                inst.opc = 0;
            }
            break;
        }
        case "movz": {
            if (match_string(&s, "movz")) {
                inst.opc = 1;
            }
            break;
        }
        case "movk": {
            if (match_string(&s, "movk")) {
                inst.opc = 3;
            }
            break;
        }
        default: {
            return false;
        }
    }

    // set the registers not included in the string

    char *opcode;
    ShiftType shift_type = 0;
    uint8_t shift_amount = 0;
    bool is_valid = parse_from(src, a_l_opcodes, opcode)
                   && skip_whitespace(&s)
                   && parse_reg(&s, &inst.rd, &inst.sf)
                   && skip_whitespace(&s)
                   && parse_immediate(&s, &inst.dp_imm.operand.wide_move_operand.imm16);
    if (!is_valid) return false;
    parse_immediate_shift(&s, "lsl", &shift_amount);
    if (!(shift_amount == 0 || shift_amount == 16 || shift_amount == 32 || shift_amount == 48)) return false;

    // parse this type of shift #<imm>{, lsl #<imm>}
    // then check its 0,16,32,48
    // parse the shift -> 0, 1, 2, 3 etc. determines hw bit.

    // set hw,imm16 value
    switch (shift_amount) {
        case 0: {
            inst.dp_imm.operand.wide_move_operand.hw = 0;
            break;
        }
        case 16: {
            inst.dp_imm.operand.wide_move_operand.hw = 1;
            break;
        }
        case 32: {
            inst.dp_imm.operand.wide_move_operand.hw = 2;
            break;
        }
        case 48: {
            inst.dp_imm.operand.wide_move_operand.hw = 3;
            break;
        }
        default: {
            return false;
        }
    }

    inst.dp_imm.operand.wide_move_operand.imm16 = imm16;

    *src = s;
    *instruction = inst;
    return true;
}


bool parse_mul(char **src, bool three_reg, Instruction *instruction) {
    // <Rd>, <Rn>, <Rm>, <Ra>

    char *s = *src;
    // write data we know from precondition
    Instruction inst = { .command_format = DP_REG, .opc = 0, .dp_reg.opr = 8,
                         .dp_reg.m = 1 };

    // save data from mnemonic
    char x;
    if (match_string(&s, "madd") || match_string(&s, "mul")) {
        x = 0;
    } else if (match_string(&s, "msub") || match_string(&s, "mneg")) {
        x = 1;
    } else { return false; }

    // check instruction string for data
    regwidth rd_width;
    regwidth rn_width;
    regwidth rm_width;
    regwidth ra_width;
    bool is_valid = skip_whitespace(&s)
                   && parse_reg(&s, &inst.rd, &rd_width)
                   && skip_whitespace(&s)
                   && parse_reg(&s, &inst.dp_reg.rn, &rn_width)
                   && (rd_width == rn_width)
                   && skip_whitespace(&s)
                   && parse_reg(&s, &inst.dp_reg.rm, &rm_width)
                   && (rn_width == rm_width); 

    // write to ra, checking if its three reg or not
    if (three_reg) {
        inst.dp_reg.operand = ZERO_REG_INDEX;
        ra_width = rm_width; // set it as the same as another width to not affect the check later
    } else {
        is_valid &= skip_whitespace(&s)
                  && parse_reg(&s, &inst.dp_reg.operand, &ra_width);
    }
    inst.dp_reg.operand |= (x << 5);

    // writing check
    if (!is_valid) { return false; }
    
    // final width check + write sign flag
    if (rm_width != ra_width) { return false; }
    inst.sf = (ra_width == _32_BIT) ? 0 : 1;

    *instruction = inst;
    return true;
}

bool parse_offset_type(char **src, Instruction *inst) {
    char *s_imm = *src;
    char *s_pre = *src;
    char *s_post = *src;
    char *s_reg = *src;
    char *s_lit = *src;

    if (match_char(&s_reg, '[')
        && parse_reg(&s, &inst.single_data_transfer.offset 
}
