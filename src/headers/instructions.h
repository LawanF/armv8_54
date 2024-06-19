// A header file describing the instruction data type.

#ifndef INSTRUCTIONS_H
#define INSTRUCTIONS_H
#include <stdint.h>

// Returns 1 if the i'th bit of n is 1 and 0 otherwise
// Uses an unsigned long to work with 32 bits
#define GET_BIT(n, i) (((n) >> (i)) & 0x1ULL)
// Applies a bitmask and returns bits from i (inclusive) to j (inclusive) of n, where i<=j,
// shifted so that bit i is bit 0 in the result
#define BITMASK(n, i, j) (((n) >> (i)) & ((0x1ULL << ((j) - (i) + 1)) - 1))
// Returns a 1 bit with i zeros to the right of it
#define FILL_BIT(i) (0x1ULL << (i))
// Negates a number 32-bit number n

// enum for specifying type of instruction
typedef enum { UNKNOWN, HALT, DP_IMM, DP_REG, SINGLE_DATA_TRANSFER, LOAD_LITERAL, BRANCH } CommandFormat;
// enum for specifying width of registers, for the sf field in Instruction
typedef enum regwidth { _32_BIT, _64_BIT } RegisterWidth;
// enum for specifying the discrete shift, for the sh field in DPImmOperand
typedef enum { ZERO_SHIFT, TWELVE_SHIFT } DiscreteShift;

typedef enum { AND, BIC, ORR, ORN, EOR, EON, ANDS, BICS } LogicType;

typedef enum { ARITH_OPERAND, WIDE_MOVE_OPERAND } DPImmOperandType;
typedef union {
    /* arithmetic instruction:
       - sh determines whether to left shift the immediate value by 12 bits
       - imm12 is an unsigned immediate value of 12 bits
       - rn is a register which is added or subtracted to for setting into rd */
    struct { DiscreteShift sh; uint8_t rn; uint16_t imm12; } arith_operand;
    /* wide move instruction: 
       - hw determines a left shift by multiple of 16 (from 0 to 48 inclusive)
       - imm16 is an unsigned immediate value of 16 bits */
    struct { uint8_t hw; uint16_t imm16; } wide_move_operand;
} DPImmOperand;

typedef enum { REGISTER_OFFSET, PRE_INDEX_OFFSET, POST_INDEX_OFFSET, UNSIGNED_OFFSET } SDTOffsetType;
typedef union {
    unsigned char xm:5; // register offset
    int16_t simm9; // pre or post index
    uint16_t imm12; // unsigned offset
} SDTOffset;

typedef enum { UNCOND_BRANCH, REGISTER_BRANCH, COND_BRANCH, } BranchOperandType;
typedef union {
    struct { int32_t simm26; } uncond_branch;
    struct { uint8_t xn; } register_branch;
    struct { uint8_t cond; int32_t simm19; } cond_branch;
} BranchOperand;

// generic instruction struct - unions for specific instruction data
typedef struct {
    CommandFormat command_format;
    // sign flag (for all types except branch)
    RegisterWidth sf;
    // opcode (for DP instructions)
    uint8_t opc;
    // destination (DP) or target (SDT or load literal) register index
    union { uint8_t rd; uint8_t rt; };
    union {
        // DP (immediate)
        struct { DPImmOperandType operand_type; DPImmOperand operand; } dp_imm;
        /* DP (register):
           - M and opr determine the type of instruction
           - rm is the multiplier (for multiply instructions)
           - rn is the multiplicand (for multiply instructions) or the left hand side of bitwise operations
           - operand contains data such as an immediate value or shift */
        struct {
            bool m; uint8_t opr; uint8_t rm;
            uint8_t operand; uint8_t rn;
        } dp_reg;
        /* single data transfer:
           - u is the unsigned offset flag
           - l determines a load rather than a store
           - xn is the base register */
        struct { bool u; bool l; uint8_t xn;
            SDTOffsetType offset_type; SDTOffset offset;
        } single_data_transfer;
        // load literal: simm19 is a signed immediate value
        struct { int32_t simm19; } load_literal;
        // branch
        struct { BranchOperandType operand_type; BranchOperand operand; } branch;
    };
} Instruction;

#endif
