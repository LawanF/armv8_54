#ifndef INSTRUCTION_CONSTANTS_H
#define INSTRUCTION_CONSTANTS_H

#include "instructions.h"

/*
 * Constants shared between instructions
 */

// binary representation of HALT instruction
#define HALT_BIN 0x8a000000
#define UNKNOWN_INSTRUCTION ((Instruction) { .command_format = UNKNOWN })
#define HALT_INSTRUCTION ((Instruction) { .command_format = HALT })

// returns bits from i to j - 1 inclusive, filled with ones
#define SET_BITS(FROM, TO) ((1ULL << (TO)) - FILL_BIT(FROM))
// returns a number sign extended from width FROM casted to intTO_t, assuming this is defined in <stdint.h>
#define SIGN_EXTEND(num, FROM, TO) ((int##TO##_t) (num | ((GET_BIT(num, FROM - 1)) ? SET_BITS(FROM, TO) : 0)))

// start and end of RD or RT
#define RD_RT_START 0
#define RD_RT_END   4

/*
 * Constants for DP instructions
 */

// for DP instructions, the sign bits are in position 31
// OPC is stored in bits 29 and 30
#define DP_SF_BIT 31
#define DP_OPC_START  29
#define DP_OPC_END    30

// for DP (immediate),
// the instruction is of format
// [ sf:1 ][ opc:2 ]100[ opi:3 ][               operand:18          ][ rd:5 ]
#define DP_IMM_MASK       0x10000000UL // 0001 0000 0000 0000 0000 0000 0000 0000
#define DP_IMM_OPI_START  23
#define DP_IMM_OPI_END    25
#define DP_IMM_MASK_START 26
#define DP_IMM_MASK_END   28

// For decoding the DP (immediate) operand:

#define ARITH_OPI 0x2
#define WIDE_MOVE_OPI 0x5
// DP immediate operands use bits 5-22
// arithmetic has format [ sh:1 ][ imm12:12 ][ rn:5 ]
#define ARITH_OP_RN_START    5
#define ARITH_OP_RN_END      9
#define ARITH_OP_IMM12_START 10
#define ARITH_OP_IMM12_END   21
#define ARITH_OP_SH_BIT      22
// wide move has format  [ hw:2    ][ imm16:16      ]
#define WIDE_MOVE_IMM16_START 5
#define WIDE_MOVE_IMM16_END   20
#define WIDE_MOVE_HW_START    21
#define WIDE_MOVE_HW_END      22

// for DP (register),
// the instruction is of format
// [ sf:1 ][ opc:2 ][ M:1 ]101[ opr:4 ][ rm:5 ][ operand: 6 ][ rn:5 ][ rd:5 ]
#define DP_REG_MASK          0x0A000000UL // 0000 1010 0000 0000 0000 0000 0000 0000
#define DP_REG_MASK_START    25
#define DP_REG_MASK_END      27
#define DP_REG_RN_START      5
#define DP_REG_RN_END        9
#define DP_REG_OPERAND_START 10
#define DP_REG_OPERAND_END   15
#define DP_REG_RM_START      16
#define DP_REG_RM_END        20
#define DP_REG_OPR_START     21
#define DP_REG_OPR_END       24
#define DP_REG_M_BIT         28

/*
 * Constants for single data transfer instructions
 */

// for both single data transfer types (including load literal),
// the sign flag is stored in bit 30 (instead of 31 for DP)
#define SDT_SF_BIT 30
// for single data transfer that is not a load literal,
// the instruction is of format
//  1[ sf:1 ]11100[ u:1 ]0[ l:1 ][ offset:12 ][ xn:5 ][ rt:5 ]
#define SDT_XN_START 5
#define SDT_XN_END   9
#define SDT_L_BIT    22
#define SDT_U_BIT    24
// test instruction matches
// [1   X    11100   X   0   X   XXXXXXXXXXXX  XXXXX   XXXXX ]
#define SDT_MASK_UPPER_BIT 31
#define SDT_MASK_LOWER_BIT 23
#define SDT_MASK_MIDDLE       0x1CUL
#define SDT_MASK_MIDDLE_START 25
#define SDT_MASK_MIDDLE_END   29
// offset uses bits 10-21
// offset 1XXXXX011010 gives register offset: 1[ xm:5      ]011010
// (i.e. combining lower mask and upper bit)
#define SDT_REGISTER_MASK_LOWER       0x1AUL // 011010
#define SDT_REGISTER_MASK_LOWER_START SDT_XN_START
#define SDT_REGISTER_MASK_LOWER_END   15
#define SDT_REGISTER_MASK_UPPER_BIT   21
// offset 0XXXXXXXXXI1 gives pre/post-index:  0[ simm9:9 ][ i:1 ]1
#define SDT_INDEX_MASK_LOWER_BIT SDT_XN_START
#define SDT_INDEX_MASK_UPPER_BIT SDT_XN_END

// For decoding the operand:

// SDT operand uses bits 10-21
// operand has format 0[ simm9:9  ]X1
#define SDT_OPERAND_START 10
#define SDT_OPERAND_END   21
// for pre- and post-index offsets,
// operand has format 0[ simm9:9  ]X1
#define SDT_INDEX_I_BIT       11
#define SDT_INDEX_MASK        FILL_BIT(10)
#define SDT_INDEX_SIMM9_START 12
#define SDT_INDEX_SIMM9_END   20
// for register offsets,
// operand has format 1[ xm:5 ]011010
#define SDT_REGISTER_MASK     ((uint32_t) 0x81A << SDT_OPERAND_START) // 1000 0001 1010
#define SDT_REGISTER_XM_START 16
#define SDT_REGISTER_XM_END   20
// for unsigned offsets,
// operand has format [  imm12:12   ] (so uses the whole operand)
#define SDT_UNSIGNED_IMM12_START SDT_OPERAND_START
#define SDT_UNSIGNED_IMM12_END   SDT_OPERAND_END

/*
 * Constants for load literal instructions
 */

// instruction of format 0[ sf:1 ]011000[ simm19:19 ][ rt:5 ]
#define LOAD_LITERAL_MASK 0x18000000UL // 0001 1000 0000 0000 0000 0000 0000 0000
#define LOAD_LITERAL_MASK_START     24
#define LOAD_LITERAL_MASK_END       29
#define LOAD_LITERAL_SIMM19_START   5
#define LOAD_LITERAL_SIMM19_END     23
#define LOAD_LITERAL_UPPER_MASK_BIT 31

/*
 * Constants for branch instructions
 */

// common bits shared across all branch instructions
// are bits 26-29 0101:            --0101-----------------------------
#define BRANCH_COMMON_MASK       0x5UL
#define BRANCH_COMMON_MASK_START 26
#define BRANCH_COMMON_MASK_END   29
// unconditional branch has format 000101[         simm26:26         ]
// bits 26-21 000101
// simm26 takes lower 26 bits
#define BRANCH_UNCOND_MASK 0x14000000UL // 0001 0100 0000 0000 0000 0000 0000 0000
#define BRANCH_UNCOND_SIMM26_START 0
#define BRANCH_UNCOND_SIMM26_END   25
#define BRANCH_UNCOND_MASK_START   26
#define BRANCH_UNCOND_MASK_END     31
// conditional branch has format   01010100[  simm19:19  ]0[ cond: 4 ]
// bits 24-31 01010100 and bit 4 0
// cond uses lower 4 bits and simm19 bits 5-23
#define BRANCH_COND_MASK 0x54000000UL // 0101 0100 0000 0000 0000 0000 0000 0000
#define BRANCH_COND_UPPER_MASK_START 24
#define BRANCH_COND_UPPER_MASK_END   31
#define BRANCH_COND_LOWER_MASK_BIT   4
#define BRANCH_COND_COND_START       0
#define BRANCH_COND_COND_END         3
#define BRANCH_COND_SIMM19_START     5
#define BRANCH_COND_SIMM19_END       23
// register branch has format      1101011000011111000000[ xn:5 ]00000
// bits 0-4 00000 and bits 10-31 are 11 0101 1000 0111 1100 0000
// xn uses bits 5-9
#define BRANCH_REG_MASK 0xD61F0000UL // 1101 0110 0001 1111 0000 0000 0000 0000
#define BRANCH_REG_MASK_LOWER_START 0
#define BRANCH_REG_MASK_LOWER_END   4
#define BRANCH_REG_MASK_UPPER_START 10
#define BRANCH_REG_MASK_UPPER_END   31
#define BRANCH_REG_XN_START         5
#define BRANCH_REG_XN_END           9

#endif
