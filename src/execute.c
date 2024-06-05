#include <stdint.h>
#include <stdlib.h>
#include "execute.h"
#include "registers.h"

static void offset_program_counter(MachineState machine_state, int32_t enc_address) {
	int64_t offset = enc_address*4;
	offset += (machine_state.program_counter.data);
    offset -= 4;
    write_program_counter(offset);
}

static void add(unsigned char rd, uint64_t rn_data, uint64_t op2, unsigned char sf) {
    uint64_t res = rn_data + op2;
    if (sf == 0) {
        write_general_registers(rd,(uint32_t)res);
    } else {
        write_general_registers(rd, res);
    }
}

static void adds(unsigned char rd, uint64_t rn_data, uint64_t op2, unsigned char sf) {
    uint64_t res = rn_data + dp_imm_imm12;
    
    if (sf == 0) { 
        res = (uint32_t)res
        set_pstate_flag('N', GET_BIT(res, 31));
    } else {
        set_pstate_flag('N', GET_BIT(res, 63));
    }
    write_general_registers(rd, res);

    if (res == 0) {
        // set zero flag to 1
        set_pstate_flag('Z', 1);
    } else {
        // set zero flag to 0
        set_pstate_flag('Z', 0);
    }

    if (res < rn_data || res < dp_imm+imm12) {
        // set carry flag to 1
        set_pstate_flag('C', 1);
    } else {
        // set carry flag to 0
        set_pstate_flag('C', 0);
    }

    if ((rn_data > 0 && op2 > 0 && res < 0) || (rn_data < 0 && op2 < 0 && res > 0)) {
        // set signed overflow flag to 1
        set_pstate_flag('V', 1);
    } else {
        // set signed overflow flag to 0
        set_pstate_flag('V', 0);
    }
}

static void sub(unsigned char rd, uint64_t rn_data, uint64_t op2, unsigned char sf) {
    uint64_t res = rn_data - op2;
    if (sf == 0) {
        write_general_registers(rd, (uint32_t)res);
    } else {
        write_general_registers(rd, res);
    }
}

static void subs(unsigned char rd, uint64_t rn_data, uint64_t op2, unsigned char sf) {
    uint64_t res = rn_data - op2;
    
    if (sf == 0) {
        res = (uint32_t)res
        set_pstate_flag('N', GET_BIT(res, 31));
    } else {
        set_pstate_flag('N', GET_BIT(res, 63));
    }
    write_general_registers(rd, res);

    if (res == 0) {
        // set zero flag to 1
        set_pstate_flag('Z', 1);
    } else {
        // set zero flag to 0
        set_pstate_flag('Z', 0);
    }

    if (res < rn_data || res < op2) {
        // set carry flag to 1
        set_pstate_flag('C', 1);
    } else {
        // set carry flag to 0
        set_pstate_flag('C', 0);
    }

    if ((rn_data < 0 && op2 > 0 && res > 0) || (rn_data > 0 && op2 < 0 && res < 0)) {
        // set signed overflow flag to 1
        set_pstate_flag('V', 1);
    } else {
        // set signed overflow flag to 0
        set_pstate_flag('V', 0);
    }
}

static void arith_inst_exec(unsigned char opc, unsigned char rd, uint64_t rn_data, uint64_t op2, unsigned char sf) {
	switch (opc) {
            case 0: {
                add(rd, rn_data, op2, sf);
                break;
            }
            case 1: {
                adds(rd, rn_data, op2, sf);
                break;
            }
            case 2: {
                sub(rd, rn_data, op2, sf);
                break;
            }
            case 3: {
                subs(rd, rn_data, op2, sf);
                break;
            }
      }
}

static void read_write_to_mem(unsigned char sdt_l, uint64_t sdt_rt, uint64_t mem_address, unsigned char sdt_sf) {
        if (sdt_l == 1) {
            // read from mem 
            // write to rt

            uint64_t data_load = readmem64(mem_address);

            // If in 32-bit mode, make upper bits 0.
            if (sdt_sf == 0) {
                data_load = (uint32_t) data_load;
            }

            write_general_registers(sdt_rt, data_load);

        } else {
            // read from rt
            // write to mem

            uint64_t data_store = (machine_state.general_registers)[sdt_rt];
            
            // Using the correct write with regards 32- or 64-bit mode.
            if (sdt_sf == 0) {
                writemem32(mem_address, data_store);
            } else {
                writemem64(mem_address, data_store);
            }
        }
}


static void halt(MachineState machine_state) {
    unsigned char *filename = get_output_file();
    print_output(machine_state, filename);
    exit(1);
}

static void movn(unsigned char dp_imm_rd, uint64_t wide_move_operand, unsigned char sf) {
    // ~OP by xor with 1111...
    // set all bits to one except imm16 bits (which these are will vary depending on if the imm16 was shifted earlier)
    // in 32 bit case upper 32 bits are all 0 (i.e. zero extended)
    // set rd to op
    wide_move_operand = ~wide_move_operand;
    if (sf == 0) {
        wide_move_operand = (uint32_t)wide_move_operand;
    }
    write_general_registers(dp_imm_rd, wide_move_operand);
}

static void movz(unsigned char dp_imm_rd, uint64_t wide_move_operand) {
    write_general_registers(dp_imm_rd, wide_move_operand);
}

static void movk(unsigned char dp_imm_rd, uint64_t wide_move_operand, unsigned char wide_move_hw, unsigned char sf) {
    // get rd data
    // mask rd data around bits that will be replaced with operand (dependent on wide_move_hw * 16)
    // and with operand bits
    // in 32 bit version zero extend to 64
    uint64_t wide_move_rd_data = (machine_state.general_registers)[dp_imm_rd].data;

    // Get top bits.
    uint64_t new_rd_data = BITMASK(wide_move_rd_data, 16 + (wide_move_hw * 16), 63) << (16 + (wide_move_hw * 16));

    // Get bottom bits.
    new_rd_data |= BITMASK(wide_move_rd_data, 0, (wide_move_hw * 16) - 1);

    // Insert operand.
    new_rd_data |= wide_move_operand;

    // 0-extend if in 32-bit mode. Check if hw is within bounds.
    if (sf == 0) {
        assert(wide_move_hw =< 1);
        new_rd_data = (uint32_t) new_rd_data;
    }

    write_general_registers(dp_imm_rd, new_rd_data);
}

static void dp_imm(MachineState machine_state, Instruction *inst) {
    DPImmOperandType dpimm_operand_type = (inst->dp_imm).operand_type;
    unsigned char dp_imm_opc = (inst->opc);
    unsigned char dp_imm_rd = (inst->rd);
    unsigned char dp_imm_sf = (inst->sf);
    switch (dpimm_operand_type) {
        case ARITH_OPERAND: {
            // get imm12, rn
            // get sh and see if need to left shift imm12
            // stack pointer (rn/rd = 11111 case handled by write to register function)
            // use opc to select specific instuction, update rd and condition (pstate) flags if needed

            uint32_t dp_imm_imm12 = (inst->dp_imm).operand.arith_operand.imm12;

            unsigned char dp_imm_rn = (inst->dp_imm).operand.arith_operand.rn;
            unsigned char dp_imm_sh = (inst->dp_imm).operand.arith_operand.sh;

            if (sh==1) {
                dp_imm_imm12 = dp_imm_imm12 << 12;
            }
            uint64_t dp_imm_rn_data = (machine_state.general_registers)[dp_imm_rn].data;

            arith_inst_exec(dp_imm_opc, dp_imm_rd, dp_imm_rn_data, dp_imm_imm12, dp_imm_sf);

            break;
        }
        case WIDE_MOVE_OPERAND: {
            switch (dp_imm_opc) {
                    // get imm16, hw
                    // operand = imm16 << (hw * 16)
                    // In the 32-bit version of the move instuction, hw can only take the values 00 or 01 (representing shifts of 0 or 16 bits)
                    // ^ separate 32 bit inst? do i need error checks for values that aren't 0 or 1

                    uint16_t wide_move_imm16 = (inst->dp_imm).operand.wide_move_operand.imm16;
                    unsigned char wide_move_hw = (inst->dp_imm).operand.wide_move_operand.hw;

                    uint32_t wide_move_operand = wide_move_imm16 << (wide_move_hw * 16);
                    case 0: {
                        movn(dp_imm_rd, wide_move_operand, dp_imm_sf);
                        break;
                    }
                    case 2: {
                        movz(dp_imm_rd, wide_move_operand);
                        break;
                    }
                    case 3: {
                        movk(dp_imm_rd, wide_move_operand);
                        break;
                    }
            }
            break;
        }    
    }
}


static void dp_reg(MachineState machine_state, Instruction *inst) {
        unsigned char dp_reg_sf = (inst->sf);
        unsigned char dp_reg_opc = (inst->opc);
        unsigned char dp_reg_m = (inst->dp_reg).m;
        unsigned char dp_reg_opr = (inst->dp_reg).dp_reg.opr;
        unsigned char dp_reg_rm = (inst->dp_reg).dp_reg.rm;
        unsigned char dp_reg_operand = (inst->dp_reg).operand;
        unsigned char dp_reg_rn = (inst->dp_reg).rn;
        unsigned char dp_reg_rd = (inst->rd);
        uint64_t dp_reg_rn_data = (machine_state.general_registers)[dp_reg_rn].data;
        uint64_t dp_reg_rm_data = (machine_state.general_registers)[dp_reg_rm].data;
        static uint64_t res;
        if (dp_reg_m == 0) {
            char dp_reg_shift = BITMASK(dp_reg_opr, 1, 2);
            // get data from registers rn, rm
            // perform shift on rm for cases 00, 01, 10, case 11 only done in logical case
            // op2 = rm shifted operand many bits
            // then execute instuction Rd = Rn (op) Op2
            
            switch (dp_reg_shift) {
                case 0: { 
                    /* lsl */ 
                    dp_reg_rm_data = dp_reg_rm_data << dp_reg_operand;
                    break;
                }
                case 1: { 
                    /* lsr */ 
                    dp_reg_rm_data = dp_reg_rm_data >> dp_reg_operand; 
                    break;
                }
                case 2: { 
                    /* asr */
                    if (dp_reg_sf == 0) {
                        dp_reg_rm_data = (((int32_t)dp_reg_rm_data) >> dp_reg_operand);
                    } else {    
                        dp_reg_rm_data = (((int64_t)dp_reg_rm_data) >> dp_reg_operand); 
                    }
                    break;
                }
            }


            if (GET_BIT(dp_reg_opr, 0) == 0) {
                // arithmetic
                // same as for dp_imm

                arith_inst_exec(dp_reg_opc, dp_reg_rd, dp_reg_rn_data, dp_reg_rm_data, dp_reg_sf);
            } else {
                // logical
                // handle shift case 11
                // handle case of N = 0,1
                // logical instuctions
                if (dp_reg_shift == 3) {
                    // ror
                    unsigned char size = dp_reg_sf ? 64 : 32;
                    if (!dp_reg_sf) dp_reg_operand &= 31; // dp_reg_operand %= 32
                    dp_reg_rm_data = (dp_reg_rm_data >> dp_reg_operand) | (dp_reg_rm_data << (size - dp_reg_operand));
                }

                if (GET_BIT(dp_reg_opr, 3) == 1) {
                    if (!dp_reg_sf) {

                    dp_reg_rm_data = ~dp_reg_rm_data;
                }

                switch (dp_reg_opc) {
                    case 0: {
                        // and / bic
                        res = dp_reg_rn_data & dp_reg_rm_data;
                        break;
                    }
                    case 1: {
                        // orr / orn
                        res = dp_reg_rn_data | dp_reg_rm_data;
                        break;
                    }
                    case 2: {
                        // eor / eon
                        res = dp_reg_rn_data ^ dp_reg_rm_data;
                        break;
                    }
                    case 3: {
                        // ands / bics
                        res = dp_reg_rn_data & dp_reg_rm_data;
                        // set flags 
                        if (!dp_reg_sf) {
                            set_pstate_flag('N', GET_BIT(res, 31));
                        } else {
                            set_pstate_flag('N', GET_BIT(res, 63));
                        }
                        if (res == 0) {
                            // set zero register Z to 1
                            set_pstate_flag('Z', 1);
                        } else {
                            // set zero register Z to 0
                            set_pstate_flag('Z', 0);
                        }
                        // set registers C and V to 0
                        set_pstate_flag('C', 0);
                        set_pstate_flag('V', 0);
                        break;
                    }
                }

                if (dp_reg_sf == 0) {
                    dp_reg_rm_data = (uint32_t)dp_reg_rm_data;
                }

                write_general_registers(dp_reg_rd, res);
            }
        } else {
            // multiply
            unsigned char multiply_x = GET_BIT(dp_reg_operand, 0);
            unsigned char multiply_ra = BITMASK(dp_reg_operand, 1, 5);

            uint64_t multiply_ra_data = (machine_state.general_registers)[multiply_ra].data;
            if (multiply_x == 0) {
                // madd
                res = multiply_ra_data + (dp_reg_rn_data * dp_reg_rm_data);
            } else {
                // msub
                res = multiply_ra_data - (dp_reg_rn_data * dp_reg_rm_data);
            }
            write_general_registers(dp_reg_rd, res);
        }
}


static void sdt(MachineState machine_state, Instruction *inst) {
    unsigned char sdt_l = (inst->single_data_transfer).l;
    unsigned char sdt_xn = (inst->single_data_transfer).xn;
    unsigned char sdt_sf = (inst->sf);
    unsigned char sdt_opc = (inst->opc);
    unsigned char sdt_rt = (inst->rt);
    uint64_t sdt_rt_data = (machine_state.general_registers)[sdt_rt].data;
    uint64_t sdt_xn_data = (machine_state.general_registers)[sdt_xn].data;
    SDTOffsetType sdt_type = (inst->single_data_transfer).offset_type;

    switch (sdt_type) {
        case REGISTER_OFFSET: {
            unsigned char sdt_xm = (inst->single_data_transfer).offset.xm;
            uint64_t sdt_xm_data =  (machine_state.general_registers)[sdt_xm].data;
            uint64_t mem_address = sdt_xm_data + sdt_xn_data;
            read_write_mem(sdt_l, sdt_rt, mem_address, sdt_sf);
            break;
        }
        case PRE_INDEX_OFFSET: {
            int16_t sdt_simm9 = (inst->single_data_transfer).offset.simm9;
            uint64_t mem_address = sdt_xn_data + sdt_simm9;
            read_write_mem(sdt_l, sdt_rt, mem_address, sdt_sf);
            write_general_registers(sdt_xn, mem_address);
            break;
        }
        case POST_INDEX_OFFSET: {
            int16_t sdt_simm9 = (inst->single_data_transfer).offset.simm9;
            read_write_mem(sdt_l, sdt_rt, sdt_xn_data, sdt_sf);
            write_general_registers(sdt_xn, sdt_xn_data + sdt_simm9);
            break;
        }
        case UNSIGNED_OFFSET: {
            uint16_t sdt_imm12 = (inst->single_data_transfer).offset.imm12;
            uint64_t uoffset = sdt_imm12 * 8;
            read_write_mem(sdt_l, sdt_rt, sdt_xn_data + uoffset, sdt_sf);
            break;
        }
    }
}

static void load_lit(MachineState machine_state, Instruction *inst) {
    uint64_t sdt_pc = (machine_state.program_counter).data;
    int32_t sdt_simm19 = (inst->load_literal).simm19;
    unsigned char sdt_sf = (inst->sf);
    read_write_mem(1, sdt_rt, sdt_pc + sdt_simm19 * 4, sdt_sf);
}

static void branch(MachineState machine_state, Instruction *inst) {
    // decrement pc when editing
    // how to specify PC when writing to machine state


    enum BRANCH_OPERAND_TYPE branch_operand_type = (inst->branch).operand_type;

    switch (branch_operand_type) {
        case UNCOND_BRANCH: {
            offset_program_counter(machine_state, (inst->branch).operand.uncond_branch.simm26);
            break;
            // use machine state function to write PC = PC + simm26*4 (sign extend to 64 bit)
        }
        case REGISTER_BRANCH: {
            // use machine state function to read register branch_xn
            // then write address in branch_xn to PC
            char register_branch_xn = (inst->branch).operand.register_branch.xn;
            write_program_counter((machine_state.general_registers)[register_branch_xn].data);
            break;
        }
        case COND_BRANCH: {
                char eval_cond = (inst->branch).operand.cond_branch.cond;
                ProcessorStateRegister branch_pstate = (machine_state.pstate);
                switch (eval_cond) {
                    case 0: {
                        if (branch_pstate.zero == 1) {
                            offset_program_counter(machine_state, (inst->branch).operand.cond_branch.simm19);
                        }
                        break;
                    }
                    case 1: {
                        if (branch_pstate.zero == 0) {
                            offset_program_counter(machine_state, (inst->branch).operand.cond_branch.simm19);
                        }
                        break;
                    }
                    case 10: {
                        if (branch_pstate.neg == branch_pstate.overflow) {
                            offset_program_counter(machine_state, (inst->branch).operand.cond_branch.simm19);
                        }
                        break;
                    }
                    case 11: {
                        if (branch_pstate.neg != branch_pstate.overflow) {
                            offset_program_counter(machine_state, (inst->branch).operand.cond_branch.simm19);
                        }
                        break;
                    }
                    case 12: {
                        if (branch_pstate.zero == 0 && branch_pstate.neg == branch_pstate.overflow) {
                            offset_program_counter(machine_state, (inst->branch).operand.cond_branch.simm19);
                        }
                        break;
                    }
                    case 13: {
                        if (!(branch_pstate.zero == 0 && branch_pstate.neg == branch_pstate.overflow)) {
                            offset_program_counter(machine_state, (inst->branch).operand.cond_branch.simm19);
                        }
                        break;
                    }
                    case 14: {
                        offset_program_counter(machine_state, (inst->branch).operand.cond_branch.simm19);
                        break;
                    }
                }
                break;
            }
    }
}

void execute(Instruction *inst) {
    if (inst == NULL) return;
    enum CommandFormat inst_command_format = inst->command_format;
    switch (inst_command_format) {
        
	    MachineState machine_state = read_machine_state();

    	case HALT: {
            halt(machine_state);
            break;
        }
        case DP_IMM: {
            dp_imm(machine_state, inst);	    
            break;
        }
        case DP_REG: {
		    dp_reg(machine_state, inst);
            break;
        }
	    case SINGLE_DATA_TRANSFER: {
            sdt(machine_state, inst);
            break;
        }
        case LOAD_LITERAL: {
            load_lit(machine_state, inst);
            break;
        }
        case BRANCH: {
            branch(machine_state, inst);
	        break;
        }
    }
}
1