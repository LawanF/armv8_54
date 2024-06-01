#include "instructions.h"

#define HALT_BIN 0x8a000000
#define UNKNOWN_INSTRUCTION ((Instruction) { .command_format = UNKNOWN }
#define HALT_INSTRUCTION ((Instruction) { .command_format = HALT }

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
