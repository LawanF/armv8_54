#include <stdint.h>

// Returns 1 if the i'th bit of n is 1 and 0 otherwise
#define GET_BIT(n, i) (((n) >> (i)) & 0x1) 
// Applies a bitmask and returns bits from i (inclusive) to j (inclusive) of n, where i<=j
#define BIT_MASK(n, i, j) (((n) >> (i)) & (0x1 << ((j) - (i) + 1) - 1))

// enum for specifying type of instruction
typedef enum { UNKNOWN; HALT; DP_IMM; DP_REG; SINGLE_DATA_TRANSFER; LOAD_LITERAL; BRANCH; } CommandFormat;

typedef enum { ARITH_OPERAND; WIDE_MOVE_OPERAND; } DPImmOperandType;
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

typedef enum { REGISTER_OFFSET; PRE_INDEX_OFFSET; POST_INDEX_OFFSET; UNSIGNED_OFFSET; } SDTOffsetType;
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

typedef enum { UNCOND_BRANCH; REGISTER_BRANCH; COND_BRANCH; } BranchOperandType;
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
            DPImmOperandType operand_type;
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
            SDTOffsetType offset_type;
            SDTOffset offset;
        } single_data_transfer;
        struct {
            int32_t simm19:19;
        } load_literal;
        struct {
            BranchOperandType operand_type;
            BranchOperand operand;
        } branch;
    }
} Instruction

CommandFormat decode_format(uint32_t inst_data) {
    if (inst_data == 0x8a000000) return HALT;
    if (BIT_MASK(inst_data, 26, 28) == 0x4) return DP_IMM;
    if (BIT_MASK(inst_data, 25, 27) == 0x5) return DP_REG;
    if (BIT_MASK(inst_data, 23, 29) == 0xE2 && GET_BIT(inst_data, 31)) return SINGLE_DATA_TRANSFER;
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

// Fills the fields of a new Instruction. A precondition is that the instruction falls into the specified type.
Instruction decode_dp_imm(uint32_t inst_data) {
    char opi = BIT_MASK(inst_data, 23, 25);
    if (opi != 0x5 && opi != 0x2) return UNKNOWN_INSTRUCTION;
    return {
        .command_format = DP_IMM,
        .sf  = GET_BIT(inst_data, 31),
        .opc = BIT_MASK(inst_data, 29, 30),
        .rd  = BIT_MASK(inst_data, 0, 4),
        .dp_imm = { .opi = opi, .operand = dp_imm_operand(opi, inst_data) }
    }
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
	    // rewrite if statement for different branch format
	    // decrement pc when editing		     
	    if ((inst->branch).operand != NULL) {
		MachineState machine_state = read_machine_state()
		uint64_t offset = simm26*4 + (machine_state 	
		write_machine_state(	
	
		// use machine state function to write PC = PC + simm26*4 (sign extend to 64 bit)
	    }
	    if ((inst->branch).branch_xn != NULL) {
                // use machine state function to read register branch_xn
                // if xn is 11111 then ignore
	        // then write address in branch_xn to PC
	    }
	    if ((inst->branch).conditional != NULL) {
	    	
	    }
	    break;
        }
    }
}

