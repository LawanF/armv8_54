#ifndef MEMORY_H
#define MEMORY_H
#include <stdint.h>

#define MEMORY_SIZE 2097152

extern void initmem();

extern void loadtomem(void *arr, size_t numbytes);

extern uint32_t readmem32(uint32_t address);

extern uint64_t readmem64(uint32_t address);

extern void writemem32(uint32_t address, uint32_t data);

extern void writemem64(uint32_t address, uint64_t data);

#endif
