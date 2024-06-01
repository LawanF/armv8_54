#include "instructions.h"
#include "instruction_constants.h"

DPImmOperand decode_dp_imm_operand(DPImmOperandType operand_type, uint32_t inst_data) {
    switch (operand_type) {
        case ARITH_OPERAND:
            return (DPImmOperand) { .arith_operand = {
                .sh    = GET_BIT(inst_data, ARITH_OP_SH_BIT),
                .imm12 = BITMASK(inst_data, ARITH_OP_IMM12_START, ARITH_OP_IMM12_END),
                .rn    = BITMASK(inst_data, ARITH_OP_RN_START,    ARITH_OP_RN_END)
            } };
        case WIDE_MOVE_OPERAND:
            return (DPImmOperand) { .wide_move_operand = {
                .hw    = BITMASK(inst_data, WIDE_MOVE_HW_START,    WIDE_MOVE_HW_END),
                .imm16 = BITMASK(inst_data, WIDE_MOVE_IMM16_START, WIDE_MOVE_IMM16_END) }
            };
    }
}
