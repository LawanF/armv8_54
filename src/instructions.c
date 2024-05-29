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
        int32_t simm26;
    } uncond_branch;
    struct {
        char xn:6;
    } register_branch;
    struct {
        char cond:4;
        int32_t simm19;
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
	    
			     break;
        }
    }
}

