#ifndef ASSEMBLE_H
#define ASSEMBLE_H

typedef struct {
    bool is_instruction;
    union {
        int32_t directive;
        Instruction inst;
    } data;
} ProgramLine;

extern int run_assembler(int argc, char **argv);

#endif
