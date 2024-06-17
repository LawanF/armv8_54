#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>
#include <ctype.h>
#include <stdint.h>
#include "parser.h"
#include "emulate_files/registers.h"

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

bool parse_from(char **src, char **tokens, char **chosen) {
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
bool parse_reg(char **src, int *index, RegisterWidth *width) {
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

    int offset_xn;
    RegisterWidth xn_width;
    int offset_xm;
    RegisterWidth xm_width;

    int uoffset;

    if (match_char(&s_reg, '[')
        && parse_reg(&s_reg, &offset_xn, &xn_width) 
        && skip_whitespace(&s_reg)
        && parse_reg(&s_reg, &offset_xm, &xm_width)
        && match_char(&s_reg, ']')
        && xn_width == xm_width) {
            inst.single_data_transfer.xn = offset_xn;
            inst.single_data_transfer.offset.xm = offset_xm;
            inst.single_data_transfer.offset_type = REGISTER_OFFSET;
            // ldr w0 [xn, xm] - REGISTER OFFSET
            // what to do about ldr w0 [xn]; case
            return true;
    } else if (match_char(&s_post, '[')
               && parse_reg(&s_post, &offset_xn, xn_width)
               && match_char(&s_post, ']')
               && skip_whitespace(&s_post)
               && parse_immediate(&s_post, uoffset)) {
            inst.single_data_transfer.xn = offset_xn;
            inst.single_data_transfer.offset.simm9 = uoffset;
            inst.single_data_transfer.offset_type = POST_INDEX_OFFSET;
            // ldr w0 [xn], #imm - post index
            return true;
    } else if (match_char(&s_pre, '[')
        && parse_reg(&s_pre, &offset_xn, &xn_width)
        && skip_whitespace(&s_pre)
        && parse_immediate(&s_pre, uoffset)
        && match_char(&s_pre, ']')
        && match_char(&s_pre, '!')) {
            inst.single_data_transfer.xn = offset_xn;
            inst.single_data_transfer.offset.simm9 = uoffset;
            inst.single_data_transfer.offset_type = PRE_INDEX_OFFSET;
            // ldr w0 [xn, #imm]! - pre index
            return true;
    } else if (match_char(&s_reg, '[')
        && parse_reg(&s_imm, &offset_xn, &xn_width)
        && skip_whitespace(&s_imm)
        && parse_immediate(&s_imm, uoffset)
        && match_char(&s_reg, ']')) {
            inst.single_data_transfer.xn = offset_xn;
            inst.single_data_transfer.offset.imm12 = uoffset;
            inst.single_data_transfer.offset_type = UNSIGNED_OFFSET;
            inst.single_data_transfer.u = 1;
            // ldr w0 [xn #imm] - unsigned offset
            return true;
    } else if (parse_immediate(&s_lit, uoffset)) {
            inst.load_literal.simm19 = uoffset;
            inst.command_format = 5;
            // ldr w0 #imm - load literal
            return true;
    } else {
        return false;
    }
}

bool parse_load(char **src, Instruction *inst) {
    char *s = *src;
    Instruction inst = { .command_format = SINGLE_DATA_TRANSFER };
    inst.single_data_transfer.u = 0;
    inst.single_data_transfer.l = 1;

    bool is_valid = match_string(&s, "ldr")
                    && skip_whitespace(&s)
                    && parse_reg(&s, &inst.rt, &inst.sf)
                    && skip_whitespace(&s)
                    && parse_offset_type(&s, *inst);

    if (!is_valid) return false;
}

bool parse_store(char **src, Instruction *inst) {
    char *s = *src;
    Instruction inst = { .command_format = SINGLE_DATA_TRANSFER };
    inst.single_data_transfer.u = 0;
    inst.single_data_transfer.l = 0;

    bool is_valid = match_string(&s, "str")
                    && skip_whitespace(&s)
                    && parse_offset_type(&s, &inst);
                    && skip_whitespace(&s)
                    && parse_reg(&s, &inst.rt, &inst.sf)

    if (!is_valid) return false;
}









