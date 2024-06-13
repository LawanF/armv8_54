#include "decode.h"
#include "instruction_constants.h"
#include "instructions.h"

/* Functions for decoding operands
 * These functions assume that the instruction is in the correct group,
 * and the instructions are valid. */

// Decodes a DP (immediate) operand, given its type.
static DPImmOperand decode_dp_imm_operand(DPImmOperandType operand_type, uint32_t inst_data) {
    DPImmOperand operand;
    switch (operand_type) {
        case ARITH_OPERAND:
            operand = (DPImmOperand) { .arith_operand = {
                .sh    = GET_BIT(inst_data, ARITH_OP_SH_BIT),
                .imm12 = BITMASK(inst_data, ARITH_OP_IMM12_START, ARITH_OP_IMM12_END),
                .rn    = BITMASK(inst_data, ARITH_OP_RN_START,    ARITH_OP_RN_END)
            } };
            break;
        case WIDE_MOVE_OPERAND:
            operand = (DPImmOperand) { .wide_move_operand = {
                .hw    = BITMASK(inst_data, WIDE_MOVE_HW_START,    WIDE_MOVE_HW_END),
                .imm16 = BITMASK(inst_data, WIDE_MOVE_IMM16_START, WIDE_MOVE_IMM16_END) }
            };
            break;
    }
    return operand;
}

// Decodes a single data transfer offset, given its type.
static SDTOffset decode_sdt_offset(SDTOffsetType offset_type, uint32_t inst_data) {
    SDTOffset offset;
    switch (offset_type) {
        case PRE_INDEX_OFFSET:
        case POST_INDEX_OFFSET: {
            uint16_t simm9_masked = BITMASK(inst_data, SDT_INDEX_SIMM9_START, SDT_INDEX_SIMM9_END);
            offset.simm9 = SIGN_EXTEND(simm9_masked, 9, 16);
            break;
        }
        case REGISTER_OFFSET:
            offset.xm    = BITMASK(inst_data, SDT_REGISTER_XM_START, SDT_REGISTER_XM_END);
            break;
        case UNSIGNED_OFFSET:
            offset.imm12 = BITMASK(inst_data, SDT_UNSIGNED_IMM12_START, SDT_UNSIGNED_IMM12_END);
    }
    return offset;
}

/* Functions for decoding instructions.
 * A precondition is that the instructions are of the specified group, and are well-formed. */

// Decodes a DP (immediate) instruction, returning it as a copy.
static Instruction decode_dp_imm(uint32_t inst_data) {
    DPImmOperandType operand_type;
    unsigned char opi = BITMASK(inst_data, DP_IMM_OPI_START, DP_IMM_OPI_END);
    switch (opi) {
        case ARITH_OPI: operand_type = ARITH_OPERAND; break;
        case WIDE_MOVE_OPI: operand_type = WIDE_MOVE_OPERAND; break;
        default: return UNKNOWN_INSTRUCTION;
    }
    return (Instruction) {
        .command_format = DP_IMM,
        .sf  = GET_BIT(inst_data, DP_SF_BIT),
        .opc = BITMASK(inst_data, DP_OPC_START, DP_OPC_END),
        .rd  = BITMASK(inst_data, RD_RT_START, RD_RT_END),
        .dp_imm = { .operand_type = operand_type, .operand = decode_dp_imm_operand(operand_type, inst_data) }
    };
}

// Decodes a DP (register) instruction, returning it as a copy.
static Instruction decode_dp_reg(uint32_t inst_data) {
    char opr = BITMASK(inst_data, DP_REG_OPR_START, DP_REG_OPR_END);
    char m = GET_BIT(inst_data, DP_REG_M_BIT);
    /* instructions of the form (M,opr) = (0,1xx0),(0,0xxx),(1,1000) are all recognised,
     * leaving (1,0xxx) and (0,1xx1) as unrecognised. */
    if ((m && !GET_BIT(opr, 3)) || (!m && GET_BIT(opr, 0) && GET_BIT(opr, 3))) {
        return UNKNOWN_INSTRUCTION;
    }
    return (Instruction) {
        .command_format = DP_REG,
        .sf  = GET_BIT(inst_data, DP_SF_BIT),
        .opc = BITMASK(inst_data, DP_OPC_START, DP_OPC_END),
        .rd  = BITMASK(inst_data, RD_RT_START, RD_RT_END),
        .dp_reg = {
            .m = m,
            .opr = opr,
            .rm      = BITMASK(inst_data, DP_REG_RM_START,      DP_REG_RM_END),
            .operand = BITMASK(inst_data, DP_REG_OPERAND_START, DP_REG_OPERAND_END),
            .rn      = BITMASK(inst_data, DP_REG_RN_START,      DP_REG_RN_END)
        }
    };
}

// Decodes a load literal instruction, returning it as a copy.
static Instruction decode_load_literal(uint32_t inst_data) {
    // sign extend simm19
    uint32_t simm19_masked = BITMASK(inst_data, LOAD_LITERAL_SIMM19_START, LOAD_LITERAL_SIMM19_END);
    return (Instruction) {
        .command_format = LOAD_LITERAL,
        .sf = GET_BIT(inst_data, SDT_SF_BIT),
        .rt = BITMASK(inst_data, RD_RT_START, RD_RT_END),
        .load_literal = { .simm19 = SIGN_EXTEND(simm19_masked, 19, 32) }
    };
}

// Decodes a branch instruction, returning it as a copy.
static Instruction decode_branch(uint32_t inst_data) {
    BranchOperandType operand_type;
    if (BITMASK(inst_data, BRANCH_UNCOND_MASK_START, BRANCH_UNCOND_MASK_END)
        == BRANCH_UNCOND_MASK >> BRANCH_UNCOND_MASK_START) {
        operand_type = UNCOND_BRANCH;
    }
    else if ((BITMASK(inst_data, BRANCH_COND_UPPER_MASK_START, BRANCH_COND_UPPER_MASK_END)
             == BRANCH_COND_MASK >> BRANCH_COND_UPPER_MASK_START)
             && !GET_BIT(inst_data, BRANCH_COND_LOWER_MASK_BIT)) {
        operand_type = COND_BRANCH;
    }
    else if (BITMASK(inst_data, BRANCH_REG_MASK_LOWER_START, BRANCH_REG_MASK_LOWER_END) == 0
             && (BITMASK(inst_data, BRANCH_REG_MASK_UPPER_START, BRANCH_REG_MASK_UPPER_END)
                 == BRANCH_REG_MASK >> BRANCH_REG_MASK_UPPER_START)) {
        operand_type = REGISTER_BRANCH;
    } else return UNKNOWN_INSTRUCTION;
    Instruction branch_inst = {
        .command_format = BRANCH,
        .branch = { .operand_type = operand_type }
    };
    BranchOperand branch_operand;
    switch (operand_type) {
        case UNCOND_BRANCH: {
            uint32_t simm26_masked = BITMASK(inst_data, BRANCH_UNCOND_SIMM26_START, BRANCH_UNCOND_SIMM26_END);
            branch_operand = (BranchOperand) { .uncond_branch =
                { .simm26 = SIGN_EXTEND(simm26_masked, 26, 32) }
            };
            break;
        }
        case COND_BRANCH: {
            uint32_t simm19_masked = BITMASK(inst_data, BRANCH_COND_SIMM19_START, BRANCH_COND_SIMM19_END);
            branch_operand = (BranchOperand) { .cond_branch =
                {
                    .cond = BITMASK(inst_data, BRANCH_COND_COND_START, BRANCH_COND_COND_END),
                    .simm19 = SIGN_EXTEND(simm19_masked, 19, 32)
                }
            };
            break;
        }
        case REGISTER_BRANCH:
            branch_operand = (BranchOperand) { .register_branch =
                { .xn = BITMASK(inst_data, BRANCH_REG_XN_START, BRANCH_REG_XN_END) }
            };
            break;
    }
    branch_inst.branch.operand = branch_operand;
    return branch_inst;
}

// Decodes a single data transfer instruction, returning it as a copy.
static Instruction decode_single_data_transfer(uint32_t inst_data) {
    SDTOffsetType offset_type;
    char u = GET_BIT(inst_data, SDT_U_BIT);
    // offset uses bits 10-21
    // when U=1, offset is used for imm12 (unsigned)
    if (u) {
        offset_type = UNSIGNED_OFFSET;
    } else if (GET_BIT(inst_data, SDT_REGISTER_MASK_UPPER_BIT)
             && (BITMASK(inst_data, SDT_REGISTER_MASK_LOWER_START, SDT_REGISTER_MASK_LOWER_END)
                 == SDT_REGISTER_MASK_LOWER)) {
        offset_type = REGISTER_OFFSET;
    } else if (!GET_BIT(inst_data, SDT_INDEX_MASK_UPPER_BIT)
               && GET_BIT(inst_data, SDT_INDEX_MASK_LOWER_BIT)) {
        // if I = 1, pre-indexed, otherwise post-indexed
        char i = GET_BIT(inst_data, SDT_INDEX_I_BIT);
        offset_type = i ? PRE_INDEX_OFFSET : POST_INDEX_OFFSET;
    } else return UNKNOWN_INSTRUCTION;

    return (Instruction) {
        .command_format = SINGLE_DATA_TRANSFER,
        .sf = GET_BIT(inst_data, SDT_SF_BIT),
        .rt = BITMASK(inst_data, RD_RT_START, RD_RT_END),
        .single_data_transfer = {
            .u = u,
            .l = GET_BIT(inst_data, SDT_L_BIT),
            .xn = BITMASK(inst_data, SDT_XN_START, SDT_XN_END),
            .offset_type = offset_type,
            .offset      = decode_sdt_offset(offset_type, inst_data)
        }
    };
}

/* Determines the format of the instruction.
 * Returns UNKNOWN if no known format is immediately known without decoding further. */
static CommandFormat decode_format(uint32_t inst_data) {
    if (inst_data == HALT_BIN) return HALT;
    // bits 26-28 100
    if (BITMASK(inst_data, DP_IMM_MASK_START, DP_IMM_MASK_END)
        == DP_IMM_MASK >> DP_IMM_MASK_START) return DP_IMM;
    // bits 25-27 101
    if (BITMASK(inst_data, DP_REG_MASK_START, DP_REG_MASK_END)
        == DP_REG_MASK >> DP_REG_MASK_START) return DP_REG;
    // bits 23-29 11100X0 and bit 31 1
    if ((BITMASK(inst_data, SDT_MASK_MIDDLE_START, SDT_MASK_MIDDLE_END)
         == SDT_MASK_MIDDLE)
        && !GET_BIT(inst_data, SDT_MASK_LOWER_BIT)
        && GET_BIT(inst_data, SDT_MASK_UPPER_BIT)) return SINGLE_DATA_TRANSFER;
    // bits 24-29 011000 and bit 31 0
    if ((BITMASK(inst_data, LOAD_LITERAL_MASK_START, LOAD_LITERAL_MASK_END)
         == LOAD_LITERAL_MASK >> LOAD_LITERAL_MASK_START)
        && !GET_BIT(inst_data, LOAD_LITERAL_UPPER_MASK_BIT)) return LOAD_LITERAL;
    // bits 26-29 0101
    if (BITMASK(inst_data, BRANCH_COMMON_MASK_START, BRANCH_COMMON_MASK_END)
        == BRANCH_COMMON_MASK) return BRANCH;
    return UNKNOWN;
}

/* Decodes an instruction from ARMv8-a.
 * If the instruction is malformed or unknown, the Instruction's command_format field will be UNKNOWN. */
Instruction decode(uint32_t inst_data) {
    switch (decode_format(inst_data)) {
        case HALT:                 return HALT_INSTRUCTION;
        case DP_IMM:               return decode_dp_imm(inst_data);
        case DP_REG:               return decode_dp_reg(inst_data);
        case SINGLE_DATA_TRANSFER: return decode_single_data_transfer(inst_data);
        case LOAD_LITERAL:         return decode_load_literal(inst_data);
        case BRANCH:               return decode_branch(inst_data);
        case UNKNOWN:
        default:                   return UNKNOWN_INSTRUCTION;
    }
}
