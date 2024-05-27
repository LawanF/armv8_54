#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#define INSTRUCTION_MEMORY_SIZE 2097152
#define TEST_PATH "add01_exp.bin"
#define BYTES_TO_PRINT 10

char instruction_memory[INSTRUCTION_MEMORY_SIZE];

void read_instr_from_file(char *filename, char *arr) {
    // Open file.
    FILE *fileptr = fopen(filename, "rb");

    // Check if file opened successfully.
    if (fileptr == NULL) {
        fprintf(stderr, "cat: can’t open %s\n", filename);
        exit(1);
    }

    char instr;
    int index = 0;
    
    // Read file byte by byte and insert into arr.
    while ((instr = getc(fileptr)) != EOF) {
        arr[index] = instr;
        index++;
    }

    // Close file.
    fclose(fileptr);
}
