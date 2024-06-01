#include <stdint.h>
#include "instructions.h"
#include "instruction_constants.h"

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

