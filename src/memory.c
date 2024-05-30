#include <string.h>
#include "memory.h"

#define MEMORY_SIZE 2097152

// Define a char[] representing the machine memory.
static unsigned char memory[MEMORY_SIZE];

/*
    Sets all values of memory to 0.
*/
void initmem() {
    memset(memory, 0, MEMORY_SIZE * sizeof(char));
}

/*
    Takes an address as uint32_t.
    Returns a pointer to the byte at that address as unsigned char.
*/
static unsigned char *fetchbyte(uint32_t address) {
    return &memory[address];
}

/*
    Takes an address as uint32_t.
    Returns 32-bits of data at that address as uint32_t.
*/
uint32_t readmem32(uint32_t address) {
    // Fetch pointer to the first byte.
    unsigned char *startbyte = fetchbyte(address);
    // Define uint32_t to return.
    uint32_t data = 0;

    // Fetch each byte, shifting them accordingly.
    for (int i = 0; i < 4; i++) {
        data |= startbyte[i] << (8 * i);
    }

    return data;
}

/*
    Takes an address as uint32_t.
    Returns 64-bits of data at that address as uint64_t.
*/
uint64_t readmem64(uint32_t address) {
    // Define uint64_t to return.
    uint64_t data = 0;

    // Fetch each data from memory using readmem32.
    for (int i = 0; i < 8; i += 4) {
        data <<= 32;
        data |= readmem32(address + 4 - i);
    }

    return data;
}

/*
    Takes an address and data as uint32_t.
    Writes 4 bytes at specified address.
*/
void writemem32(uint32_t address, uint32_t data) {
    // Fetch pointer to the first byte.
    unsigned char *startbyte = fetchbyte(address);

    // Write to memory byte-by-byte.
    for (int i = 0; i < 4; i++) {
        startbyte[i] = data;
        data >>= 8;
    }
}

/*
    Takes an address as uint32_t and 64-bit data as uint64_t.
    Writes 8 bytes at specified address.
*/
void writemem64(uint32_t address, uint64_t data) { 
    // Write to memory using writemem32.
    for (int i = 0; i < 8; i += 4) {
        writemem32(address + i, data);
        data >>= 32;
    }
}
