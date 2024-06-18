#include <stdbool.h>
#include <assert.h>
#include "encode.h"
#include "instruction_constants.h"

#define FAIL_ENCODE() assert(-1); return 0

/* Functions for encoding operands.
 * These functions assume that the instruction is in the correct group,
 * and the instructions are valid. */

// Encodes a DP (immediate) operand, given a reference to an Instruction.
static uint32_t encode_dp_imm_operand(const Instruction *inst) {
    DPImmOperandType operand_type = inst->dp_imm.operand_type;
    DPImmOperand operand = inst->dp_imm.operand;
    switch (operand_type) {
        case ARITH_OPERAND:
            return (ARITH_OPI << DP_IMM_OPI_START)
                   | ((uint32_t) operand.arith_operand.sh << ARITH_OP_SH_BIT)
                   | ((uint32_t) operand.arith_operand.imm12 << ARITH_OP_IMM12_START)
                   | ((uint32_t) operand.arith_operand.rn    << ARITH_OP_RN_START);
        case WIDE_MOVE_OPERAND:
            return (ARITH_OPI << DP_IMM_OPI_START)
                   | ((uint32_t) operand.wide_move_operand.hw    << WIDE_MOVE_HW_START)
                   | ((uint32_t) operand.wide_move_operand.imm16 << WIDE_MOVE_IMM16_START);
        default: FAIL_ENCODE();
    }
}

// Encodes a single data transfer offset, given a reference to an Instruction.
static uint32_t encode_sdt_offset(const Instruction *inst) {
    SDTOffsetType offset_type = inst->single_data_transfer.offset_type;
    SDTOffset offset = inst->single_data_transfer.offset;
    char i = 0; // if i = 1, then pre-indexed, otherwise post_indexed
    switch (offset_type) {
        case PRE_INDEX_OFFSET: i = 1;
        case POST_INDEX_OFFSET:
            return SDT_INDEX_MASK
                   | ((uint32_t) i << SDT_INDEX_I_BIT)
                   // since the value is signed, we need to apply a mask to remove negated bits
                   | BITMASK((uint32_t) offset.simm9 << SDT_INDEX_SIMM9_START, 0, SDT_INDEX_SIMM9_END);
        case REGISTER_OFFSET:
            return SDT_REGISTER_MASK
                   | ((uint32_t) offset.xm << SDT_REGISTER_XM_START);
        case UNSIGNED_OFFSET:
            return (uint32_t) offset.imm12 << SDT_UNSIGNED_IMM12_START;
        default: FAIL_ENCODE();
    }
}

/* Functions for encoding instructions.
 * A precondition is that the instructions are of the specified group, and are well-formed. */

// Encodes a DP (immediate) instruction, given a reference to an Instruction.
static uint32_t encode_dp_imm(const Instruction *inst) {
    unsigned char opi = 0;
    switch (inst->dp_imm.operand_type) {
        case ARITH_OPERAND:     opi = ARITH_OPI;     break;
        case WIDE_MOVE_OPERAND: opi = WIDE_MOVE_OPI; break;
        default: FAIL_ENCODE(); // if the operand type is unknown
    }
    return DP_IMM_MASK
           | ((uint32_t) inst->sf  << DP_SF_BIT)
           | ((uint32_t) inst->opc << DP_OPC_START)
           | ((uint32_t) opi       << DP_IMM_OPI_START)
           | encode_dp_imm_operand(inst)
           | ((uint32_t) inst->rd  << RD_RT_START);
}

// Encodes a DP (register) instruction, given a reference to an Instruction.
static uint32_t encode_dp_reg(const Instruction *inst) {
    return DP_REG_MASK
           | ((uint32_t) inst->rd             << RD_RT_START)
           | ((uint32_t) inst->dp_reg.rn      << DP_REG_RN_START)
           | ((uint32_t) inst->dp_reg.operand << DP_REG_OPERAND_START)
           | ((uint32_t) inst->dp_reg.rm      << DP_REG_RM_START)
           | ((uint32_t) inst->dp_reg.opr     << DP_REG_OPR_START)
           | ((uint32_t) inst->dp_reg.m       << DP_REG_M_BIT)
           | ((uint32_t) inst->opc            << DP_OPC_START)
           | ((uint32_t) inst->sf             << DP_SF_BIT);
}

// Encodes a single data transfer instruction, given a reference to an Instruction.
static uint32_t encode_single_data_transfer(const Instruction *inst) {
    uint32_t encoded_offset = encode_sdt_offset(inst);
    if (!encoded_offset) FAIL_ENCODE();
    return FILL_BIT(SDT_REGISTER_MASK_UPPER_BIT)
           | ((uint32_t) SDT_MASK_MIDDLE << SDT_MASK_MIDDLE_START)
           // no need to add the mask lower bit as it is zero
           | ((uint32_t) inst->rt << RD_RT_START)
           | ((uint32_t) inst->single_data_transfer.xn << SDT_XN_START)
           | encoded_offset
           | ((uint32_t) inst->single_data_transfer.l << SDT_L_BIT)
           | ((uint32_t) inst->single_data_transfer.u << SDT_XN_START)
           | ((uint32_t) inst->sf << SDT_SF_BIT);
}

// Encodes a load literal instruction, given a reference to an Instruction.
static uint32_t encode_load_literal(const Instruction *inst) {
    // mask simm19 to remove leading 1 bits for negative values
    uint32_t simm19_masked = BITMASK(inst->load_literal.simm19, 0, 18);
    return LOAD_LITERAL_MASK
           | ((uint32_t) inst->rt << RD_RT_START)
           | (simm19_masked << LOAD_LITERAL_SIMM19_START)
           | ((uint32_t) inst->sf << SDT_SF_BIT);
}

// Encodes a branch instruction, given a reference to an Instruction.
static uint32_t encode_branch(const Instruction *inst) {
    switch (inst->branch.operand_type) {
        case UNCOND_BRANCH: {
            // need to remove leading 1 bits in case simm26 is negative
            uint32_t simm26_masked = BITMASK(inst->branch.operand.uncond_branch.simm26, 0, 25);
            return BRANCH_UNCOND_MASK
                   | (simm26_masked << BRANCH_UNCOND_SIMM26_START);
        }
        case COND_BRANCH: {
            uint32_t cond = inst->branch.operand.cond_branch.cond;
            uint32_t simm19_masked = BITMASK(inst->branch.operand.cond_branch.simm19, 0, 18);
            return BRANCH_COND_MASK
                   | (simm19_masked << BRANCH_COND_SIMM19_START)
                   | (cond << BRANCH_COND_COND_START);
        }
        case REGISTER_BRANCH: {
            uint32_t xn = inst->branch.operand.register_branch.xn;
            return BRANCH_REG_MASK
                   | (xn << BRANCH_REG_XN_START);
        }
        default: FAIL_ENCODE();
    }
}

/* Encodes an instruction stored at the given pointer into ARMv8-a.
 * Assumes that the instruction is well-formed.
 * If the instruction is unknown, returns zero. */
uint32_t encode(const Instruction *inst) {
    switch (inst->command_format) {
        case HALT:                 return HALT_BIN;
        case DP_IMM:               return encode_dp_imm(inst);
        case DP_REG:               return encode_dp_reg(inst);
        case SINGLE_DATA_TRANSFER: return encode_single_data_transfer(inst);
        case LOAD_LITERAL:         return encode_load_literal(inst);
        case BRANCH:               return encode_branch(inst);
        case UNKNOWN:              return 0;
        default:                   FAIL_ENCODE();
    }
}
