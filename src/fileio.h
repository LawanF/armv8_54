#ifndef FILEIO_H
#define FILEIO_H
#include "emulate_files/registers.h"

extern void store_file_to_mem(char *filename);

extern void print_output(MachineState *machine_state, char *filename);

#endif


