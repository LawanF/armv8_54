#include <stdint.h>

// Returns 1 if the i'th bit of n is 1 and 0 otherwise
#define GET_BIT(n, i) (((n) >> (i)) & 0x1) 
// Applies a bitmask and returns bits from i (inclusive) to j (inclusive) of n, where i<=j
#define BIT_MASK(n, i, j) (((n) >> (i)) & (0x1 << ((j) - (i) + 1) - 1))

// enum for specifying type of instruction
typedef enum { UNKNOWN; HALT; DP_IMM; DP_REG; SINGLE_DATA_TRANSFER; LOAD_LITERAL; BRANCH; } CommandFormat;

typedef enum { ARITH_OPERAND; WIDE_MOVE_OPERAND; } DPImmOperandType;
typedef union {
    struct { char sh:1; uint16_t imm12:12; char rn:5; } arith_operand;
    struct { char hw:2; uint16_t imm16; } wide_move_operand;
} DPImmOperand;

typedef enum { REGISTER_OFFSET; PRE_INDEX_OFFSET; POST_INDEX_OFFSET; UNSIGNED_OFFSET; } SDTOffsetType;
typedef union {
    char xm:5; // register offset
    int16_t simm9:9; // pre or post index
    uint16_t imm12:12; // unsigned offset
} SDTOffset;

typedef enum { UNCOND_BRANCH; REGISTER_BRANCH; COND_BRANCH; } BranchOperandType;
typedef union {
    struct { int32_t simm26:26; } uncond_branch;
    struct { char xn:6; } register_branch;
    struct { char cond:4; int32_t simm19:19; } cond_branch;
} BranchOperand;

// generic instruction struct - unions for specific instruction data
typedef struct {
    CommandFormat command_format;
    char sf:1;
    char opc:2;
    union { char rd:5; char rt:5; };
    union {
        struct {} empty_inst;
        struct { DPImmOperandType operand_type; DPImmOperand operand; } dp_imm;
        struct { char m:1; char opr:4; char rm:5; char operand:6; char rn:5; } dp_reg;
        struct { char u:1; char l:1; char xn:5;
            SDTOffsetType offset_type; SDTOffset offset;
        } single_data_transfer;
        struct { int32_t simm19:19; } load_literal;
        struct { BranchOperandType operand_type; BranchOperand operand; } branch;
    }
} Instruction

CommandFormat decode_format(uint32_t inst_data) {
    if (inst_data == 0x8a000000) return HALT;
    // bits 26-28 100
    if (BIT_MASK(inst_data, 26, 28) == 0x4) return DP_IMM;
    // bits 25-27 101
    if (BIT_MASK(inst_data, 25, 27) == 0x5) return DP_REG;
    // bits 23-29 11100X0 and bit 31 1
    if (BIT_MASK(inst_data, 25, 29) == 0x1C 
        && !GET_BIT(inst_data, 23)
        && GET_BIT(inst_data, 31)) return SINGLE_DATA_TRANSFER;
    if (BIT_MASK(inst_data, 24, 29) == Ox18 && !GET_BIT(inst_data, 31)) return LOAD_LITERAL;
    if (BIT_MASK(inst_data, 26, 29) == Ox5) return BRANCH;
    return UNKNOWN;
}

#define UNKNOWN_INSTRUCTION { .command_format = UNKNOWN, .empty_instruction = {} }

// Returns an operand of the given type. A precondition is that the instruction is in the correct group.
DPImmOperand dp_imm_operand(DPImmOperandType operand_type, uint32_t inst_data) {
    switch (operand_type) {
        case ARITH_OPERAND:
            return { .arith_operand = { .sh    = GET_BIT(inst_data, 22),
                                        .imm12 = BIT_MASK(inst_data, 10, 21),
                                        .rn    = BIT_MASK(inst_data, 5, 9) } };
        case WIDE_MOVE_OPERAND:
            return { .wide_move_operand = { .hw    = BIT_MASK(inst_data, 21, 22),
                                            .imm16 = BIT_MASK(inst_data, 5, 20) } };
    }
}
SDTOffset sdt_offset(SDTOffsetType offset_type, uint32_t inst_data) {
    switch (offset_type) {
        case PRE_INDEX_OFFSET: 
        case POST_INDEX_OFFSET:
            return { .simm9 = BIT_MASK(inst_data, 12, 20) };
        case REGISTER_OFFSET:
            return { .xm    = BIT_MASK(inst_data, 16, 20) };
        case UNSIGNED_OFFSET:
            return { .imm12 = BIT_MASK(inst_data, 10, 21) };
    }
}
// Fills the fields of a new Instruction. A precondition is that the instruction falls into the specified type.
Instruction decode_dp_imm(uint32_t inst_data) {
    DPImmOperandType operand_type;
    char opi = BIT_MASK(inst_data, 23, 25);
    switch (opi) {
        case 0x5: operand_type = ARITH_OPERAND; break;
        case 0x2: operand_type = WIDE_MOVE_OPERAND; break;
        default: return UNKNOWN_INSTRUCTION;
    }
    return {
        .command_format = DP_IMM,
        .sf  = GET_BIT(inst_data, 31),
        .opc = BIT_MASK(inst_data, 29, 30),
        .rd  = BIT_MASK(inst_data, 0, 4),
        .dp_imm = { .operand_type = operand_type, .operand = dp_imm_operand(opi, inst_data) }
    };
}
Instructon decode_dp_reg(uint32_t inst_data) {
    char opr = BIT_MASK(inst_data, 21, 24);
    char m = GET_BIT(inst_data, 28);
    /* instructions of the form (M,opr) = (0,1xx0),(0,0xxx),(1,1000) are all recognised,
     * leaving (1,0xxx) and (0,1xx1) as unrecognised. */
    if ((m && !GET_BIT(opr, 3)) || (!m && GET_BIT(opr, 0) && GET_BIT(opr, 3))) {
        return UNKNOWN_INSTRUCTION;
    }
    return {
        .command_format = DP_REG,
        .sf  = GET_BIT(inst_data, 31),
        .opc = BIT_MASK(inst_data, 29, 30),
        .rd  = BIT_MASK(inst_data, 0, 4),
        .dp_reg = { .m = m,
                    .opr = opr, 
                    .operand = BIT_MASK(inst_data, 10, 15), 
                    .rn = BIT_MASK(inst_data, 5, 9) } };
}
Instruction decode_single_data_transfer(uint32_t inst_data) {
    SDTOffsetType offset_type;
    // bits 10-21 1XXXXX011010
    if (GET_BIT(inst_data, 21) && BIT_MASK(inst_data, 10, 15) == 0x1A) {
        offset_type = REGISTER_OFFSET;
    // bits 10-21 0XXXXXXXXXI1
    } else if (!GET_BIT(inst_data, 21) && GET_BIT(inst_data, 10)) {
        // if I = 1, pre-indexed, otherwise post-indexed
        offset_type = GET_BIT(inst_data, 11) ? PRE_INDEXED_OFFSET : POST_INDEXED_OFFSET;
    } else offset_type = UNSIGNED_OFFSET;
    return {
        .command_format = SINGLE_DATA_TRANSFER,
        .sf = GET_BIT(inst_data, 30),
        .rt = BIT_MASK(inst_data, 0, 4),
        .u  = GET_BIT(inst_data, 24),
        .single_data_transfer = {
            .u = GET_BIT(inst_data, 24),
            .l = GET_BIT(inst_data, 22),
            .xn = BIT_MASK(inst_data, 10, 21),
            .offset_type = offset_type,
            .offset = sdt_offset(offset_type, inst_data)
        }
    };
}

void offset_program_counter(MachineState *alter_machine_state, int32_t enc_address) {
	int64_t offset = enc_address*4;
	offset += (machine_state->program_counter.data);
        offset -= 4;
        write_machine_state(program_counter, offset);
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
            switch (dpimm_operand_type) {
	    	case ARITH_OPERAND: {
		    // get imm12, rn
		    // get sh and see if need to left shift imm12
		    // stack pointer (rn/rd = 11111 case handled by write to register function)
		    // use opc to select specific instruction, update rd and condition (pstate) flags if needed
		
		    int32_t dp_imm_imm12 = (inst->operand).arith_operand.imm12;
		    char dp_imm_rn:5 = (inst->operand).arith_operand.rn;
		    char dp_imm_sh:1 = (inst->operand).arith_operand.sh;

		    if (sh==1) {
		        dp_imm_imm12 = dp_imm_imm12 << 12;
		    }

	            switch (dp_imm_opc) {
		        case 0: {
			    write_general_registers(dp_imm_rd, (machine_state->general_registers)[dp_imm_rn].data + dp_imm_imm12);		
			    break;
			}
		        case 1: {
			    // INT32 ? OR INT64 - HOW ARE WE HANDLING REGISTER SIZE
			    int32_t rn_data = (machine_state->general_registers)[dp_imm_rn].data		
			    int32_t result = rn_data + dp_imm_imm12;
			    write_general_registers(dp_imm_rd, result);
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
			    if ((rn_data > 0 && dp_imm_imm12 > 0 && res < 0) || (rn_data < 0 && dp_imm_imm12 < 0 && res > 0)) {
				// set signed overflow flag to 1
			    } else if ((rn_data < 0 && dp_imm_imm12 > 0 && res > 0) || (rn_data > 0 && dp_imm_imm12 < 0 && res > 0)) {
			        // set signed overflow flag to 1
			    } else {
			        // set signed overflow flag to 0
			    }

			    // HOW ARE WE HANDLING SIGNED / UNSIGNED INTEGERS
			    // figure out how we are handling the size of the register - is the result meant to be stored as 32 or 64 bit int
			    break;
			}
			case 2: {
			    write_general_registers(dp_imm_rd, (machine_state->general_registers)[dp_imm_rn].data - dp_imm_imm12);
			    break;
			}
			case 3: {
			    int32_t result = (machine_state->general_registers)[dp_imm_rn].data - dp_imm_imm12;
                            write_general_registers(dp_imm_rd, result);
			    break;
			}
		    }
	            break;
	        }
	        case WIDE_MOVE_OPERAND: {
		    switch (dp_imm_opc) {
                        case 0: {
                            break;
                        }             
                        case 2: {
                            break;
                        }
                        case 3: {
                            break;
                        }       
                    }
		    break;
		}
	    }	    
            break;
        }
        case DP_REG: {

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
				write_machine_state(program_counter, (machine_state->general_registers)[register_branch_xn].data);
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

