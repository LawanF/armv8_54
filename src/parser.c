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
#include "emulate_files/instruction_constants.h"

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

bool parse_from(char **src, const char * const tokens[], int *index) {
    bool result = true;
    for (int i = 0; tokens[i] != NULL; i++) {
        if (match_string(src, tokens[i])) {
            // write the token index chosen
            *index = i;
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

/** Parses an immediate value: a string with an unsigned integer imm,
 * setting `dest` to be this value if parsing succeeds.
 * @returns true if and only if parsing succeeds
 */
bool parse_immediate(char **src, uint32_t *dest) {
    char *s = *src;
    uint32_t val;
    bool is_int =
        (match_string(&s, "0x") && parse_uint(&s, &val, /* base= */ 16)) // hex
        || parse_uint(&s, &val, /* base= */ 10);
    if (!is_int) return false;
    *src = s;
    *dest = val;
    return true;
}

/** Parses a signed immediate value: a string with a signed integer imm,
 * setting `dest` to be this value if parsing succeeds.
 * @returns true if and only if parsing succeeds
 */
bool parse_signed_immediate(char **src, int32_t *dest) {
    char *s = *src;
    int32_t val;
    bool is_int =
        (match_string(&s, "0x") && parse_int(&s, &val, /* base= */ 16)) // hex
        || parse_int(&s, &val, /* base= */ 10);
    if (!is_int) return false;
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

void set_offset(Instruction *inst, uint32_t cur_pos, uint32_t target_pos) {
    // calculate offset
    int32_t offset = target_pos - cur_pos;
    bool type_valid = true;
    LiteralInstr type =
        (inst->command_format == LOAD_LITERAL) ? LOAD :
        (inst->command_format == BRANCH) ? (
            (inst->branch.operand_type == COND_BRANCH) ? COND :
            (inst->branch.operand_type == UNCOND_BRANCH) ? UNCOND
            : (type_valid = false)
        ) : (type_valid = false);
    if (!type_valid) return;

    // set offset
    switch (type) {
        case COND:
            inst->branch.operand.cond_branch.simm19 = offset;
            break;
        case UNCOND:
            inst->branch.operand.uncond_branch.simm26 = offset;
            break;
        case LOAD:
            inst->load_literal.simm19 = offset;
            break;
    }
}

bool parse_literal(char **src, uint32_t cur_pos, Instruction *inst, SymbolTable known_table, SymbolTable unknown_table) {
    // takes in a literal and based on the instruction, saves the data to the instruction
    // whitespace is skipped before entering this function

    char *s = *src;
    bool is_valid;
    
    uint32_t target;
    // check if the literal is an immediate value
    if (parse_immediate(&s, &target)) {
        // continue to set offset
        set_offset(inst, cur_pos, target);
    } else if (symtable_contains(known_table, s)) { // check if the literal is a label that exists in the symbol table
        // label exists in the symbol table
        symtable_get(known_table, s, &target);
        // continue to set offset
        set_offset(inst, cur_pos, target);
    } else { // either the label is unknown or the line is invalid 
        // parse the next word
        char *label = strtok(s, " :");
        // check if addition failed
        if (!multi_symtable_add(unknown_table, label, cur_pos)) return false;
    }
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
                && match_char(&s, '#')
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
                && match_char(&s, '#')
                && parse_immediate(&s, &amount)
                && (amount < 64);
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
 * Note that the opcode for this instruction will need to be set separately.
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
    bool success = match_char(&s, '#')
                && parse_immediate(&s, &immediate)
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

/** Parses the register form of an < op2 > operand, a
 * string of the form "Rn{, [shift] #[imm]}", and fills in the corresponding
 * Note that the opcode for this instruction will need to be set separately.
 * @returns true if and only if the operand matches, and parsing succeeds.
 */
static bool parse_reg_op2(
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
    // where the xx bits form the shift type.
    // For a logical instruction, it is the same but with the MSB of opr as 0.
    inst.dp_reg.m = 0;
    inst.dp_reg.opr = shift_type << 1;
    inst.dp_reg.rn = rn;
    inst.dp_reg.rm = rm;
    inst.dp_reg.operand = shift_amount;
    // set instruction and string pointer
    *src = s;
    *instruction = inst;
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
    Instruction inst;
    char *s = *src;
    ArithOpc opc;
    bool success;
    // mnemonics and the aliases they map to
    const char *const add_types[]  = {"add", NULL};
    const char *const adds_types[] = {"adds", "cmn", NULL};
    const char *const sub_types[]  = {"sub", "neg", NULL};
    const char *const subs_types[] = {"cmp", "subs", "negs", NULL};
    // set opcode
    // choose this order so that "adds" is checked before "add"
    // in order to select the correct opcode
    int i;
    const char *mnemonic;
    if      (parse_from(&s, adds_types, &i)) { opc = ADDS; mnemonic = adds_types[i]; }
    else if (parse_from(&s, add_types,  &i)) { opc = ADD;  mnemonic = add_types[i];  }
    else if (parse_from(&s, subs_types, &i)) { opc = SUBS; mnemonic = subs_types[i]; }
    else if (parse_from(&s, sub_types,  &i)) { opc = SUB;  mnemonic = sub_types[i];  }
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
           || parse_reg_op2(&s, rn, &inst);
    if (!success) return false;
    // set rd and opc
    inst.rd = rd;
    inst.opc = opc;
    switch (inst.command_format) {
        case DP_IMM: break; // handled already
        case DP_REG: {
            // set opr to have MSB as 1
            inst.dp_reg.opr |= 1 << (DP_REG_OPR_END - DP_REG_OPR_START);
        }
        default: return false;
    }
    *src = s;
    *instruction = inst;
    return true;
}

typedef enum { AND, BIC, ORR, ORN, EOR, EON, ANDS, BICS } LogicType;

/** Parses an instruction of one of the the forms:
 * [ands|and|bics|bic|eor|eon|orr|orn] Rd, Rn, < op2 >
 * [tst|mvn] Rn, < op2 >
 * [mov] Rd, Rm
 * @returns true (and writes to `instruction`) if and only if parsing succeeds
 */
bool parse_logical(char **src, Instruction *instruction) {
    Instruction inst = *instruction;
    char *s = *src;
    bool is_valid;
    // Reverse the logic types so that no substrings are checked before the actual string.
    const char * const rev_logic_types[] = { "bics", "ands", "eon", "eor", "orn", "orr", "bic", "and", NULL };
    const char * const logic_types[] = { "and", "bic", "orr", "orn", "eor", "eon", "ands", "bics", NULL };

    int logic_type_int;
    LogicType logic_type;

    // The boolean values determine whether the corresponding register will be read.
    // For aliases, some registers are zero, and so the register will not need to have a value parsed.
    bool rd_bool = false; bool rn_bool = false; bool op2_bool = false; bool rm_bool = false;    

    // all logical instructions are DP (register)
    inst.command_format = DP_REG;

    if (parse_from(&s, rev_logic_types, &logic_type_int)) {
        // reverse logic_type: remove NULL termination when measuring count
        logic_type = (LogicType) logic_type_int;
        logic_type = (ARRAY_LEN(rev_logic_types) - 1) - logic_type;
        rd_bool = rn_bool = op2_bool = true;
    } else if (match_string(&s, "mvn")) {
        // "mvn rd, <op2>" is an alias for "orn rd, rzr, <op2>"
        // set zero register
        inst.dp_reg.rn = ZERO_REG_INDEX;
        logic_type = ORN; 
        rd_bool = op2_bool = true;
    } else if (match_string(&s, "mov")) {
        // "mov rd, rm" is an alias for "orr rd, rzr, rm"
        // since there is no op2, set zero register and zero shift
        inst.dp_reg.rn = ZERO_REG_INDEX;
        inst.dp_reg.operand = 0;
        logic_type = ORR; 
        rd_bool = rm_bool = true;
    } else if (match_string(&s, "tst")) {
        // "tst rn, <op2>" is an alias for "ands rzr, rn, <op2>"
        inst.rd = ZERO_REG_INDEX;
        logic_type = ANDS;
        rn_bool = op2_bool = true;
    }

    RegisterWidth cur_width;
    uint8_t rn;
    is_valid = skip_whitespace(&s)
            && parse_reg(&s, (rd_bool ? &inst.rd : &inst.dp_reg.rn), &inst.sf)
            && match_char(&s, ',')
            && skip_whitespace(&s)
            && (rn_bool ? parse_reg(&s, &rn, &cur_width)
                          && cur_width == inst.sf
                        : true) 
            && (rm_bool ? match_char(&s, ',')
                          && skip_whitespace(&s)
                          && parse_reg(&s, &inst.dp_reg.rm, &cur_width)
                          && cur_width == inst.sf
                        : true)
            && (op2_bool ? match_char(&s, ',')
                           && skip_whitespace(&s)
                           && parse_reg_op2(&s, rn, &inst)
                        : true);
    if (!is_valid) return false;

    switch (inst.command_format) {
        case DP_REG: {
            inst.dp_reg.m = 0;
            bool dp_reg_n = logic_type % 2 == 0;
            inst.dp_reg.opr |= (int) dp_reg_n;
            inst.opc = logic_type >> 1;
            break;
        }
        default:
            // the instruction type is invalid
            return false;
    }
    // parsing success
    *src = s;
    *instruction = inst;
    return true;
}

bool parse_mov_dp_imm(char **src, Instruction *instruction) {
    // movk, movn, movz
    // <Rd>, #<imm>{, lsl #<imm>}

    char *s = *src;
    Instruction inst = { .command_format = DP_IMM };
    inst.dp_imm.operand_type = WIDE_MOVE_OPERAND;
    // write data from mnemonic

    bool valid_mnemonic = true;
    inst.opc = match_string(&s, "movn") ? 0
             : match_string(&s, "movz") ? 1
             : match_string(&s, "movk") ? 3
             : (valid_mnemonic = false);
    if (!valid_mnemonic) return false;

    // set the registers not included in the string

    char *opcode;
    ShiftType shift_type = 0;
    uint8_t shift_amount = 0;
    uint32_t immediate;
    bool is_valid = skip_whitespace(&s)
                 && parse_reg(&s, &inst.rd, &inst.sf)
                 && skip_whitespace(&s)
                 && match_char(&s, '#')
                 && parse_immediate(&s, &immediate)
                 && immediate < INT16_MAX;
    if (!is_valid) return false;
    // set imm16
    inst.dp_imm.operand.wide_move_operand.imm16 = immediate;
    parse_immediate_shift(&s, &shift_type, &shift_amount);
    // parse this type of shift #<imm>{, lsl #<imm>}
    // then check its 0,16,32,48
    // parse the shift -> 0, 1, 2, 3 etc. determines hw bit.
    if (!(shift_type == LSL
          && (shift_amount == 0
              || shift_amount == 16
              || shift_amount == 32
              || shift_amount == 48))) return false;
    inst.dp_imm.operand.wide_move_operand.hw = shift_amount / 16;
    inst.dp_imm.operand.wide_move_operand.imm16 = immediate;

    *src = s;
    *instruction = inst;
    return true;
}


bool parse_mul(char **src, Instruction *instruction) {
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
    RegisterWidth rd_width;
    RegisterWidth rn_width;
    RegisterWidth rm_width;
    RegisterWidth ra_width;
    bool is_valid = skip_whitespace(&s)
        && parse_reg(&s, &inst.rd, &rd_width)
        && match_char(&s, ',')
        && skip_whitespace(&s)
        && parse_reg(&s, &inst.dp_reg.rn, &rn_width)
        && match_char(&s, ',')
        && (rd_width == rn_width)
        && skip_whitespace(&s)
        && parse_reg(&s, &inst.dp_reg.rm, &rm_width)
        && match_char(&s, ',')
        && (rn_width == rm_width);

    // write to ra, checking if its three reg or not
    char *check = s;
    skip_whitespace(&check);
    if (match_char(&check, '\0')) {
        inst.dp_reg.operand = ZERO_REG_INDEX;
        ra_width = rm_width; // set it as the same as another width to not affect the check later
    } else {
        is_valid = is_valid
                && skip_whitespace(&s)
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

bool parse_offset_type(
    char **src,
    Instruction *instruction,
    uint32_t cur_pos,
    SymbolTable known_table,
    SymbolTable unknown_table
) {
    char *s_imm = *src;
    char *s_pre = *src;
    char *s_post = *src;
    char *s_reg = *src;
    char *s_lit = *src;
    char *s_lit_label = *src;

    uint8_t offset_xn;
    RegisterWidth xn_width;
    uint8_t offset_xm;
    RegisterWidth xm_width;

    int32_t simm;
    uint32_t imm;

    Instruction inst = *instruction;
    inst.command_format = SINGLE_DATA_TRANSFER;

    // register offset: [<Xn>, <Xm>]
    if (match_char(&s_reg, '[')
        && parse_reg(&s_reg, &offset_xn, &xn_width) 
        && match_char(&s_reg, ',')
        && skip_whitespace(&s_reg)
        && parse_reg(&s_reg, &offset_xm, &xm_width)
        && match_char(&s_reg, ']')
        && xn_width == xm_width) {
        inst.single_data_transfer.xn = offset_xn;
        inst.single_data_transfer.offset.xm = offset_xm;
        inst.single_data_transfer.offset_type = REGISTER_OFFSET;
        // ldr w0 [xn, xm] - REGISTER OFFSET
        // what to do about ldr w0 [xn]; case
        *instruction = inst;
        *src = s_reg;
        return true;
    }
    // post-index offset: [<Xn>], #<simm>
    else if (match_char(&s_post, '[')
        && parse_reg(&s_post, &offset_xn, &xn_width)
        && match_char(&s_post, ']')
        && match_char(&s_post, ',')
        && skip_whitespace(&s_post)
        && match_char(&s_post, '#')
        && parse_signed_immediate(&s_post, &simm)) {
        inst.single_data_transfer.xn = offset_xn;
        inst.single_data_transfer.offset.simm9 = simm;
        inst.single_data_transfer.offset_type = POST_INDEX_OFFSET;
        // ldr w0 [xn], #imm - post index
        *instruction = inst;
        *src = s_post;
        return true;
    }
    // pre-index offset: [<Xn>, #<simm>]!
    else if (match_char(&s_pre, '[')
        && parse_reg(&s_pre, &offset_xn, &xn_width)
        && match_char(&s_pre, ',')
        && skip_whitespace(&s_pre)
        && parse_signed_immediate(&s_pre, &simm)
        && match_char(&s_pre, ']')
        && match_char(&s_pre, '!')) {
        inst.single_data_transfer.xn = offset_xn;
        inst.single_data_transfer.offset.simm9 = simm;
        inst.single_data_transfer.offset_type = PRE_INDEX_OFFSET;
        // ldr w0 [xn, #imm]! - pre index
        *instruction = inst;
        *src = s_pre;
        return true;
    }
    // unsigned offset [<Xn>, #<imm>]
    else if (match_char(&s_reg, '[')
        && parse_reg(&s_imm, &offset_xn, &xn_width)
        && match_char(&s_imm, ',')
        && skip_whitespace(&s_imm)
        && match_char(&s_imm, '#')
        && parse_immediate(&s_imm, &imm)
        && match_char(&s_reg, ']')) {
        inst.single_data_transfer.xn = offset_xn;
        inst.single_data_transfer.offset.imm12 = imm;
        inst.single_data_transfer.offset_type = UNSIGNED_OFFSET;
        inst.single_data_transfer.u = 1;
        // ldr w0 [xn #imm] - unsigned offset
        *instruction = inst;
        *src = s_imm;
        return true;
    }
    // unsigned offset [Xn]
    else if (match_char(&s_reg, '[')
        && parse_reg(&s_reg, &offset_xn, &xn_width)
        && match_char(&s_reg, ']')) {
        inst.single_data_transfer.xn = offset_xn;
        inst.single_data_transfer.offset.imm12 = 0;
        inst.single_data_transfer.offset_type = UNSIGNED_OFFSET;
        inst.single_data_transfer.u = 1;
        *instruction = inst;
        *src = s_reg;
        return true;
    }
    // load literal or immediate address
    else {
        inst.command_format = LOAD_LITERAL;
        if (parse_literal(&s_lit, cur_pos, &inst, known_table, unknown_table)) {
            *instruction = inst;
            *src = s_lit;
            return true;
        }
        return false;
    }
}

bool parse_load_store(char **src, Instruction *instruction, uint32_t cur_pos, SymbolTable known_table, SymbolTable unknown_table) {
    char *s = *src;
    Instruction inst = { .command_format = SINGLE_DATA_TRANSFER };
    inst.single_data_transfer.u = 0;

    if (match_string(&s, "ldr")) {
        inst.single_data_transfer.l = 1;
    } else if (match_string(&s, "str")) {
        inst.single_data_transfer.l = 0;
    } else {
        return false;
    }

    bool is_valid = skip_whitespace(&s)
                    && parse_reg(&s, &inst.rt, &inst.sf)
                    && skip_whitespace(&s)
                    && parse_offset_type(&s, &inst, cur_pos, known_table, unknown_table);

    if (!is_valid) return false;

    *instruction = inst;
}

bool parse_b(char **src, Instruction *instruction, uint32_t cur_pos, SymbolTable known_table, SymbolTable unknown_table) {
    // <literal>

    char *s = *src;
    // write data we know from precondition
    Instruction inst = { .command_format = BRANCH };

    // write data from mnemonic
    LiteralInstr lit_type;
    int mnemonic_index;
    if (match_string(&s, "b.") && parse_from(&s, branch_conds, &mnemonic_index)) {
        inst.branch.operand_type = COND_BRANCH;
        lit_type = COND;
        inst.branch.operand.cond_branch.cond = mnemonic_index;
    } else if (match_string(&s, "b")) {
        inst.branch.operand_type = UNCOND_BRANCH;
        lit_type = UNCOND;
    } else return false;
    if (!skip_whitespace(&s)) return false;

    bool literal_valid = parse_literal(&s, cur_pos, &inst, known_table, unknown_table);
    if (!literal_valid) return false;
    // write data from the rest of the instruction
    *src = s;
    *instruction = inst;
    return true;
}

bool parse_br(char **src, Instruction *instruction) {
    // xn

    char *s = *src;
    Instruction inst = { .command_format = BRANCH };
    // check we're on the right mnemonic
    if (match_string(&s, "br")) {
        inst.branch.operand_type = REGISTER_BRANCH;
    } else { return false; }

    // save register
    RegisterWidth width;
    bool is_valid = skip_whitespace(&s)
                    && parse_reg(&s, &inst.branch.operand.register_branch.xn, &width);
    if (!is_valid) { return false; }

    *instruction = inst;
    return true;
}

bool parse_label(char **src, SymbolTable table, uint32_t cur_pos) {
    // takes in labels and adds them to the symbol table
    char *s = *src;
    // parse the label
    char *label = strtok(s, ":");
    // add to the symbol table -- check in the multimap?
    bool add_success = single_symtable_set(table, label, cur_pos);
    return add_success;
}


bool parse_directive(char **src, int32_t *dest) {
    // x
    char *s = *src;
    int32_t value;
    // check we're on the right mnemonic
    bool success = match_string(&s, ".int")
                && skip_whitespace(&s)
                && parse_signed_immediate(&s, &value);
    if (!success) return false;
    // write to destination
    *src = s;
    *dest = value;
    return true;
}

bool parse_instruction(char **src, Instruction *instruction, uint32_t cur_pos, SymbolTable known_table, SymbolTable unknown_table) {
    char *s = *src;
    Instruction inst;
    bool is_valid = parse_add_sub(&s, &inst)
                    || parse_mov_dp_imm(&s, &inst)
                    || parse_mul(&s, &inst)
                    || parse_load_store(&s, &inst, cur_pos, known_table, unknown_table)
                    || parse_b(&s, &inst, cur_pos, known_table, unknown_table)
                    || parse_br(&s, &inst);
    *instruction = inst;
    return is_valid;
}

