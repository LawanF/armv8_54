#include <stdint.h>
#include "instructions.h"
#include "instruction_constants.h"

/* Functions for decoding and encoding operands
 * These functions assume that the instruction is in the correct group,
 * and the instructions are valid. */

uint32_t encode_dp_imm_operand(const Instruction *inst) {
    DPImmOperandType operand_type = inst->dp_imm.operand_type;
    DPImmOperand operand = inst->dp_imm.operand;
    switch (operand_type) {
        // operand uses bits 5-22
        case ARITH_OPERAND:
            return ARITH_OPI
                   | ((uint32_t) operand.arith_operand.sh << ARITH_OP_SH_BIT)
                   | ((uint32_t) operand.arith_operand.imm12 << ARITH_OP_IMM12_START)
                   | ((uint32_t) operand.arith_operand.rn    << ARITH_OP_RN_START);
        case WIDE_MOVE_OPERAND:
            return WIDE_MOVE_OPI
                   | ((uint32_t) operand.wide_move_operand.hw    << WIDE_MOVE_HW_START)
                   | ((uint32_t) operand.wide_move_operand.imm16 << WIDE_MOVE_IMM16_START);
    }
}

SDTOffset decode_sdt_offset(SDTOffsetType offset_type, uint32_t inst_data) {
    switch (offset_type) {
        case PRE_INDEX_OFFSET: 
        case POST_INDEX_OFFSET:
            return (SDTOffset) { .simm9 = BITMASK(inst_data, SDT_INDEX_SIMM9_START, SDT_INDEX_SIMM9_END) };
        case REGISTER_OFFSET:
            return (SDTOffset) { .xm    = BITMASK(inst_data, SDT_REGISTER_XM_START, SDT_REGISTER_XM_END) };
        case UNSIGNED_OFFSET:
            return (SDTOffset) { .imm12 = BITMASK(inst_data, SDT_UNSIGNED_IMM12_START, SDT_UNSIGNED_IMM12_END) };
    }
}

uint32_t encode_sdt_offset(const Instruction *inst) {
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
    }
}

/* Functions for decoding and encoding instructions.
 * A precondition is that the instructions are of the specified group, and are well-formed. */

Instruction decode_dp_imm(uint32_t inst_data) {
    DPImmOperandType operand_type;
    char opi = BITMASK(inst_data, DP_IMM_OPI_START, DP_IMM_OPI_END);
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
        .dp_imm = { .operand_type = operand_type, .operand = decode_dp_imm_operand(opi, inst_data) }
    };
}

uint32_t encode_dp_imm(const Instruction *inst) {
    DPImmOperand operand = inst->dp_imm.operand;
    char opi;
    switch (inst->dp_imm.operand_type) {
        case ARITH_OPERAND:     opi = ARITH_OPI;     break;
        case WIDE_MOVE_OPERAND: opi = WIDE_MOVE_OPI; break;
    }
    return DP_IMM_MASK
           | ((uint32_t) inst->sf  << DP_SF_BIT)
           | ((uint32_t) inst->opc << DP_OPC_START)
           | ((uint32_t) opi       << DP_IMM_OPI_START)
           | encode_dp_imm_operand(inst)
           | ((uint32_t) inst->rd  << RD_RT_START);
}

Instruction decode_dp_reg(uint32_t inst_data) {
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

uint32_t encode_dp_reg(const Instruction *inst) {
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

Instruction decode_single_data_transfer(uint32_t inst_data) {
    SDTOffsetType offset_type;
    char u = GET_BIT(inst_data, SDT_U_BIT);
    // offset uses bits 10-21
    // when U=1, offset is used for imm12 (unsigned)
    if (u) {
        offset_type = UNSIGNED_OFFSET;
    }
    else if (GET_BIT(inst_data, SDT_REGISTER_MASK_UPPER_BIT)
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

uint32_t encode_single_data_transfer(const Instruction *inst) {
    return FILL_BIT(SDT_REGISTER_MASK_UPPER_BIT)
           | ((uint32_t) SDT_MASK_MIDDLE << SDT_MASK_MIDDLE_START)
           // no need to add the mask lower bit as it is zero
           | ((uint32_t) inst->rt << RD_RT_START)
           | ((uint32_t) inst->single_data_transfer.xn << SDT_XN_START)
           | encode_sdt_offset(inst)
           | ((uint32_t) inst->single_data_transfer.l << SDT_L_BIT)
           | ((uint32_t) inst->single_data_transfer.u << SDT_XN_START)
           | ((uint32_t) inst->sf << SDT_SF_BIT);
}

Instruction decode_load_literal(uint32_t inst_data) {
    return (Instruction) {
        .command_format = LOAD_LITERAL,
        .sf = GET_BIT(inst_data, SDT_SF_BIT),
        .rt = BITMASK(inst_data, RD_RT_START, RD_RT_END),
        .load_literal = {
            .simm19 = BITMASK(inst_data, LOAD_LITERAL_SIMM19_START, LOAD_LITERAL_SIMM19_END)
        }
    };
}

uint32_t encode_load_literal(const Instruction *inst) {
    // mask simm19 to remove leading 1 bits for negative values
    uint32_t simm19_masked = BITMASK(inst->load_literal.simm19, 0, 18);
    return LOAD_LITERAL_MASK
           | ((uint32_t) inst->rt << RD_RT_START)
           | (simm19_masked << LOAD_LITERAL_SIMM19_START)
           | ((uint32_t) inst->sf << SDT_SF_BIT);
}

Instruction decode_branch(uint32_t inst_data) {
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
        case UNCOND_BRANCH: 
            branch_operand = (BranchOperand) { .uncond_branch =
                { .simm26 = BITMASK(inst_data, BRANCH_UNCOND_SIMM26_START, BRANCH_UNCOND_SIMM26_END) }
            };
            break;
        case COND_BRANCH:
            branch_operand = (BranchOperand) { .cond_branch =
                {
                    .cond = BITMASK(inst_data, BRANCH_COND_COND_START, BRANCH_COND_COND_END),
                    .simm19 = BITMASK(inst_data, BRANCH_COND_SIMM19_START, BRANCH_COND_SIMM19_END)
                }
            };
            break;
        case REGISTER_BRANCH:
            branch_operand = (BranchOperand) { .register_branch =
                { .xn = BITMASK(inst_data, BRANCH_REG_XN_START, BRANCH_REG_XN_END) }
            };
            break;
    }
    branch_inst.branch.operand = branch_operand;
    return branch_inst;
}

uint32_t encode_branch(const Instruction *inst) {
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
    }
}

/* Determines the format of the instruction.
 * Returns UNKNOWN if no known format is immediately known without decoding further. */
CommandFormat decode_format(uint32_t inst_data) {
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
        case HALT:                 return (Instruction) { .command_format = HALT };
        case DP_IMM:               return decode_dp_imm(inst_data);
        case DP_REG:               return decode_dp_reg(inst_data);
        case SINGLE_DATA_TRANSFER: return decode_single_data_transfer(inst_data);
        case LOAD_LITERAL:         return decode_load_literal(inst_data);
        case BRANCH:               return decode_branch(inst_data);
        case UNKNOWN:              return UNKNOWN_INSTRUCTION;
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
    }
}

static void offset_program_counter(MachineState *alter_machine_state, int32_t enc_address) {
	int64_t offset = enc_address*4;
	offset += (machine_state->program_counter.data);
        offset -= 4;
        write_program_counter(offset);
}

static void arith_instr_exec(char opc:2, char rd:5, uint32_t rn_data, uint32_t op2) {
	switch (opc) {
			static uint32_t res;
                        case 0: {
                            write_general_registers(rd, rn_data + op2);
                            break;
                        }
                        case 1: {
                            // INT32 ? OR INT64 - HOW ARE WE HANDLING REGISTER SIZE


                            res = rn_data + dp_imm_imm12;


                            write_general_registers(rd, res);


                            if (res < 0) {
                                // set sign flag to 1
                            } else {
                                // set sign flag to 0
                            }
                            if (res == 0) {
                                // set zero flag to 1
                            } else {
                                // set zero flag to 0
                            }
                            if (res < rn_data || res < dp_imm+imm12) {
                                // set carry flag to 1
                            } else {
                                // set carry flag to 0
                            }
                            if ((rn_data > 0 && op2 > 0 && res < 0) || (rn_data < 0 && op2 < 0 && res > 0)) {
                                // set signed overflow flag to 1
                            } else {
                                // set signed overflow flag to 0
                            }

                            // HOW ARE WE HANDLING SIGNED / UNSIGNED INTEGERS
                            // figure out how we are handling the size of the register - is the result meant to be stored as 32 or 64 bit int
                            break;
                        }
                        case 2: {
                            write_general_registers(rd, rn_data - op2);
                            break;
                        }
                        case 3: {
                            // INT32 ? OR INT64 - HOW ARE WE HANDLING REGISTER SIZE
                            res = rn_data - op2;
                            write_general_registers(rd, res);
                            if (res < 0) {
                                // set sign flag to 1
                            } else {
                                // set sign flag to 0
                            }
                            if (res == 0) {
                                // set zero flag to 1
                            } else {
                                // set zero flag to 0
                            }
                            if (res < rn_data || res < op2) {
                                // set carry flag to 1
			    } else {
                                // set carry flag to 0
                            }
                            if ((rn_data < 0 && op2 > 0 && res > 0) || (rn_data > 0 && op2 < 0 && res < 0)) {
                                // set signed overflow flag to 1
                            } else {
                                // set signed overflow flag to 0
                            }
                            break;
                        }
                    }
}

void execute(Instruction *inst) {
    if (inst == NULL) return;
    enum CommandFormat inst_command_format = inst->command_format;
    switch (inst_command_format) {
        
	MachineState *machine_state = read_machine_state();

	case HALT: {

            break;
        }
        case DP_IMM: {
	    DPImmOperandType dpimm_operand_type = (inst->dp_imm).operand_type;
	    char dp_imm_opc:2 = (inst->opc);
	    char dp_imm_rd:5 = (inst->rd);
	    char dp_imm_sf:1 = (inst->sf);
            switch (dpimm_operand_type) {
	    	case ARITH_OPERAND: {
		    // get imm12, rn
		    // get sh and see if need to left shift imm12
		    // stack pointer (rn/rd = 11111 case handled by write to register function)
		    // use opc to select specific instruction, update rd and condition (pstate) flags if needed
	
		    uint32_t dp_imm_imm12 = (inst->dp_imm).operand.arith_operand.imm12;
			
		    char dp_imm_rn:5 = (inst->dp_imm).operand.arith_operand.rn;
		    char dp_imm_sh:1 = (inst->dp_imm).operand.arith_operand.sh;

		    if (sh==1) {
		        dp_imm_imm12 = dp_imm_imm12 << 12;
		    }
		    uint32_t dp_imm_rn_data = (machine_state->general_registers)[dp_imm_rn].data;

		    arith_instr_exec(dp_imm_opc, dp_imm_rd, dp_imm_rn_data, dp_imm_imm12);

	            break;
	        }
	        case WIDE_MOVE_OPERAND: {
		    switch (dp_imm_opc) {
			// get imm16, hw
                        // operand = imm16 << (hw * 16)
                        // In the 32-bit version of the move instruction, hw can only take the values 00 or 01 (representing shifts of 0 or 16 bits)
                        // ^ separate 32 bit instr? do i need error checks for values that aren't 0 or 1

                        uint16_t wide_move_imm16 = (inst->dp_imm).operand.wide_move_operand.imm16;
                        char wide_move_hw:2 = (inst->dp_imm).operand.wide_move_operand.hw;

                        uint32_t wide_move_operand = wide_move_imm16 << (wide_move_hw * 16);
                        case 0: {
			    // ~OP by xor with 1111... 
			    // set all bits to one except imm16 bits (which these are will vary depending on if the imm16 was shifted earlier)
			    // in 32 bit case upper 32 bits are all 0 (i.e. zero extended)
			    // set rd to op
			    wide_move_operand = NEGATE_MASK(wide_move_operand);
			    write_general_registers(dp_imm_rd, wide_move_hw);
                            break;
                        }             
                        case 2: {
			    write_general_registers(dp_imm_rd, wide_move_hw);
                            break;
                        }
                        case 3: {
			    // get rd data
			    // mask rd data around bits that will be replaced with operand (dependent on hw * 16) 
			    // and with operand bits
			    // in 32 bit version zero extend to 64
			    uint32_t wide_move_rd_data = (machine_state->general_registers)[dp_imm_rd].data;
			    if (hw == 0) {
			       wide_move_rd_data = BIT_MASK(wide_move_rd_data, 16, 31) << 16;
			    } else if (hw == 1) {
			       wide_move_rd_data = BIT_MASK(wide_move_rd_data, 0, 15);
			    }
			    wide_move_rd_data = wide_move_rd_data & wide_move_operand;
			    write_general_registers(dp_imm_rd, wide_move_hw);
                            break;
                        }       
                    }
		    break;
		}
	    }	    
            break;
        }
        case DP_REG: {
	    char dp_reg_sf:1 = (inst->sf);
	    char dp_reg_opc:2 = (inst->opc);
	    char dp_reg_m:1 = (inst->dp_reg).m;
	    char dp_reg_opr:4 = (inst->dp_reg).dp_reg.opr;
	    char dp_reg_rm:5 = (inst->dp_reg).dp_reg.rm;
	    char dp_reg_operand:6 = (inst->dp_reg).operand;
	    char dp_reg_rn:5 = (inst->dp_reg).rn;
	    char dp_reg_rd:5 = (inst->rd);
	    uint32_t dp_reg_rn_data = (machine_state->general_registers)[dp_reg_rn].data;
            uint32_t dp_reg_rm_data = (machine_state->general_registers)[dp_reg_rm].data;
	    static uint32_t res;
	    if (dp_reg_m == 0) {
		char dp_reg_shift = (GET_BIT(dp_reg_opr, 1) << 1) & (GET_BIT(dp_reg_opr, 2));
		// get data from registers rn, rm
		// perform shift on rm for cases 00, 01, 10, case 11 only done in logical case
		// op2 = rm shifted operand many bits
		// then execute instruction Rd = Rn (op) Op2
		
		
		switch (dp_reg_shift) {
			case 0: { dp_reg_rm_data = dp_reg_rm_data << dp_reg_operand }
			case 1: { dp_reg_rm_data = dp_reg_rm_data >> dp_reg_operand }
			case 2: { dp_reg_rm_data = (((int32_t)dp_reg_rm_data) >> dp_reg_operand) }
		}

	        if (GET_BIT(dp_reg_opr, 0) == 0) {
		    // arithmetic
		    // same as for dp_imm
		    
		    arith_instr_exec(dp_reg_opc, dp_reg_rd, dp_reg_rn_data, dp_reg_rm_data);
		    

		} else {
		    // logical
		    // handle shift case 11
		    // handle case of N = 0,1
		    // logical instructions
		    if (dp_reg_shift == 3) {
			uint32_t right_shift = dp_reg_rm_data >> dp_reg_operand;
    			uint32_t rotate_bits = dp_reg_rm_data << (32-dp_reg_operand);
    			dp_reg_rm_data  = right_shift | rotate_bits;
		    }

		    if (GET_BIT(dp_reg_opr, 3) == 1) {
			    NEGATE_MASK(dp_reg_rm_data);
		    }

		    switch (dp_reg_opc) {
			    case 0: {
				res = dp_reg_rn_data & dp_reg_rm_data;
				break;
			    }
			    case 1: {
				res = dp_reg_rn_data | dp_reg_rm_data;
				break;
			    }
			    case 2: {
				res = dp_reg_rn_data ^ dp_reg_rm_data;
				break;
			    }
			    case 3: {
				res = dp_reg_rn_data & dp_reg_rm_data;
				// set flags 
				if (res < 0) {
					// set sign flag N to 1
				} else {
					// set sign flag N to 0
				}
				if (res == 0) {
					// set zero register Z to 1
				} else { 
					// set zero register Z to 0
				}
				// set registers C and V to 0
				break;
			    }
		    }
		    write_general_registers(dp_reg_rd, res);
		}		
	    } else {
	        // multiply
		char multiply_x:1 = GET_BIT(dp_reg_operand, 0);
		char multiply_ra:5 = BIT_MASK(dp_reg_operand, 1, 5);

		uint32_t multiply_ra_data = (machine_state->general_registers)[multiply_ra].data;
		if (multiply_x == 0) {
			res = multiply_ra_data + (dp_reg_rn_data * dp_reg_rm_data);
		} else {
			res = multiply_ra_data - (dp_reg_rn_data * dp_reg_rm_data);
		}
		write_general_registers(dp_reg_rd, res);
	    }
		   
            break;
        }
        case SINGLE_DATA_TRANSFER: {

            break;
        }
        case LOAD_LITERAL: {

            break;
        }
        case BRANCH: {
	    // decrement pc when editing
	    // how to specify PC when writing to machine state
	   

	    enum BRANCH_OPERAND_TYPE branch_operand_type = (inst->branch).operand_type;

	    switch (branch_operand_type) {

		    case UNCOND_BRANCH: {
			offset_program_counter(*machine_state, (inst->branch).operand.uncond_branch.simm26);
			break;
			// use machine state function to write PC = PC + simm26*4 (sign extend to 64 bit)
		    }
		    case REGISTER_BRANCH: {
			// use machine state function to read register branch_xn
			// if xn is 11111 then ignore
			// then write address in branch_xn to PC
			char register_branch_xn:5 = (inst->branch).operand.register_branch.xn;
			if (register_branch_xn != 31) {
				write_program_counter((machine_state->general_registers)[register_branch_xn].data);
			}
			break;
		    }
		    case COND_BRANCH: {
			char eval_cond:4 = (inst->branch).operand.cond_branch.cond;
			ProcessorStateRegister branch_pstate = (machine_state->pstate);
			switch (eval_cond) {
			    case 0: {
				if (branch_pstate.zero == 1) { 
				    offset_program_counter(*machine_state, (inst->branch).operand.cond_branch.simm19);
				}				
			    }
			    case 1: {
				if (branch_pstate.zero == 0) {
				    offset_program_counter(*machine_state, (inst->branch).operand.cond_branch.simm19);
				}
			    }
			    case 10: {
				if (branch_pstate.neg == branch_pstate.overflow) {
				    offset_program_counter(*machine_state, (inst->branch).operand.cond_branch.simm19);
				}
			    }	    
			    case 11: {
				if (branch_pstate.neg != branch_pstate.overflow) {
				    offset_program_counter(*machine_state, (inst->branch).operand.cond_branch.simm19);
				}
			    }	     
			    case 12: {
				if (branch_pstate.zero == 0 && branch_pstate.neg == branch_pstate.overflow) {
				    offset_program_counter(*machine_state, (inst->branch).operand.cond_branch.simm19);
				}
			    }	     
			    case 13: {
				if (!(branch_pstate.zero == 0 && branch_pstate.neg == branch_pstate.overflow)) {
				    offset_program_counter(*machine_state, (inst->branch).operand.cond_branch.simm19);
				}
			    }
			    case 14: {
				offset_program_counter(*machine_state, (inst->branch).operand.cond_branch.simm19);
			    }	     
			}
			break;
	    }
	    break;
        }
    }
}

