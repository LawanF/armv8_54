#include <stdint.h>

// Returns 1 if the i'th bit of n is 1 and 0 otherwise
#define GET_BIT(n, i) (((n) >> (i)) & 0x1) 
// Applies a bitmask and returns bits from i (inclusive) to j (inclusive) of n, where i<=j
#define BIT_MASK(n, i, j) (((n) >> (i)) & (0x1 << ((j) - (i) + 1) - 1))

// enum for specifying type of instruction
typedef enum { UNKNOWN; HALT; DP_IMM; DP_REG; SINGLE_DATA_TRANSFER; LOAD_LITERAL; BRANCH; } CommandFormat;

typedef union {
    struct {
        char sh:1;
        uint16_t imm12:12;
        char rn:5;
    } arith_operand;
    struct {
        char hw:2;
        uint16_t imm16;
    } wide_move_operand;
} DPImmOperand;

typedef union {
    struct {
        char xm:4;
    } register_offset;
    struct {
        int16_t simm9:9;
    } pre_post_index;
    struct {
        uint16_t imm12:12;
    } unsigned_offset;
} SDTOffset;

typedef union {
    struct {
        int32_t simm26:26;
    } uncond_branch;
    struct {
        char xn:6;
    } register_branch;
    struct {
        char cond:4;
        int32_t simm19:19;
    } cond_branch;
} BranchOperand;

// generic instruction struct - unions for specific instruction data
typedef struct {
    CommandFormat command_format;
    char sf:1;
    char opc:2;
    union {
        char rd:5;
        char rt:5;
    };
    union {
        struct {} empty_inst;
        struct {
            char opi:3;
            DPImmOperand operand;
        } dp_imm;
        struct {
            char opr:4;
            char rm:5;
            char operand:6;
            char rn:5;
        } dp_reg;
        struct {
            char l:1;
            char xn:5;
            SDTOffset offset;
        } single_data_transfer;
        struct {
            int32_t simm19:19;
        } load_literal;
        struct {
            BranchOperand operand;
        } branch;
    }
} Instruction

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
        case HALT: {

            break;
        }
        case DP_IMM: {

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
	    
	    MachineState *machine_state = read_machine_state();

	    if ((inst->branch).operand.uncond_branch != NULL) {
		offset_program_counter(*machine_state, (inst->branch).operand.uncond_branch.simm26);
		// use machine state function to write PC = PC + simm26*4 (sign extend to 64 bit)
	    }
	    if ((inst->branch).operand.register_branch != NULL) {
                // use machine state function to read register branch_xn
                // if xn is 11111 then ignore
	        // then write address in branch_xn to PC
		int8_t xn = (inst->branch).operand.register_branch.xn;
		if (xn != 31) {
			write_machine_state(program_counter, (machine_state->general_registers)[xn]);
		}
	    }
	    if ((inst->branch).operand.cond_branch != NULL) {
	    	int8_t eval_cond = (inst->branch).operand.cond_branch.cond;
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
	    }
	    break;
        }
    }
}

