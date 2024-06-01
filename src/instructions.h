// A header file describing the instruction data type.

// enum for specifying type of instruction
typedef enum { UNKNOWN, HALT, DP_IMM, DP_REG, SINGLE_DATA_TRANSFER, LOAD_LITERAL, BRANCH } CommandFormat;

typedef enum { ARITH_OPERAND, WIDE_MOVE_OPERAND } DPImmOperandType;
typedef union {
    /* arithmetic instruction:
       - sh determines whether to left shift the immediate value by 12 bits
       - imm12 is an unsigned immediate value of 12 bits
       - rn is a register which is added or subtracted to for setting into rd */
    struct { unsigned char sh:1; uint16_t imm12:12; unsigned char rn:5; } arith_operand;
    /* wide move instruction: 
       - hw determines a left shift by multiple of 16
       - imm16 is an unsigned immediate value of 16 bits */
    struct { unsigned char hw:2; uint16_t imm16; } wide_move_operand;
} DPImmOperand;

typedef enum { REGISTER_OFFSET, PRE_INDEX_OFFSET, POST_INDEX_OFFSET, UNSIGNED_OFFSET } SDTOffsetType;
typedef union {
    unsigned char xm:5; // register offset
    int16_t simm9:9; // pre or post index
    uint16_t imm12:12; // unsigned offset
} SDTOffset;

typedef enum { UNCOND_BRANCH, REGISTER_BRANCH, COND_BRANCH, } BranchOperandType;
typedef union {
    struct { int32_t simm26:26; } uncond_branch;
    struct { unsigned char xn:5; } register_branch;
    struct { unsigned char cond:4; int32_t simm19:19; } cond_branch;
} BranchOperand;

// generic instruction struct - unions for specific instruction data
typedef struct {
    CommandFormat command_format;
    // sign flag (for all types except branch)
    unsigned char sf:1;
    // opcode (for DP instructions)
    unsigned char opc:2;
    // destination (DP) or target (SDT or load literal) register index
    union { unsigned char rd:5; unsigned char rt:5; };
    union {
        // DP (immediate)
        struct { DPImmOperandType operand_type; DPImmOperand operand; } dp_imm;
        /* DP (register):
           - M and opr determine the type of instruction
           - rm is the multiplier (for multiply instructions)
           - rn is the multiplicand (for multiply instructions) or the left hand side of bitwise operations
           - operand contains data such as an immediate value or shift */
        struct {
            unsigned char m:1; unsigned char opr:4; unsigned char rm:5;
            unsigned char operand:6; unsigned char rn:5;
        } dp_reg;
        /* single data transfer:
           - u is the unsigned offset flag
           - l determines a load rather than a store
           - xn is the base register */
        struct { unsigned char u:1; unsigned char l:1; unsigned char xn:5;
            SDTOffsetType offset_type; SDTOffset offset;
        } single_data_transfer;
        // load literal: simm19 is a signed immediate value
        struct { int32_t simm19:19; } load_literal;
        // branch
        struct { BranchOperandType operand_type; BranchOperand operand; } branch;
    };
} Instruction;
