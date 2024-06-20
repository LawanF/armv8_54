#include <stdio.h>
#include <string.h>
#include "../headers/memory.h"

#define BYTE_BITS 8
#define WORD_BITS 32
#define WORD_BYTES 4

// Define a char[] representing the machine memory.
// Data is stored in little-endian.
static unsigned char memory[MEMORY_SIZE];

/*
    Clears memory, setting all values to 0.
*/
void initmem(void) {
    memset(memory, 0, MEMORY_SIZE * sizeof(char));
}

/*
    Loads an array into memory using memcpy.
    Used to load instructions to memory.
*/
void loadtomem(void *arr, uint32_t numbytes) {
    if (numbytes > MEMORY_SIZE) {
        fprintf(stderr, "loadtomem: number of bytes exceeds memory.");
    }
    memcpy(memory, arr, numbytes);
}

/*
    Takes a 21-bit address as uint32_t.
    Returns a pointer to the byte at that address as unsigned char.
*/
static unsigned char *fetchbyte(uint32_t address) {
    return &memory[address];
}

/*
    Takes a 21-bit address as uint32_t.
    Returns 32 bits (4 bytes) of data at that address as uint32_t.
*/
uint32_t readmem32(uint32_t address) {
    // Fetch pointer to the first byte.
    unsigned char *startbyte = fetchbyte(address);
    // Define uint32_t to return.
    uint32_t data = 0;

    // Fetch each byte, shifting them accordingly.
    for (int i = 0; i < WORD_BYTES; i++) {
        data |= startbyte[i] << (BYTE_BITS * i);
    }

    return data;
}

/*
    Takes an address as uint32_t.
    Returns 64 bits (8 bytes) of data at that address as uint64_t.
*/
uint64_t readmem64(uint32_t address) {
    // Define uint64_t to return.
    uint64_t data = 0;

    // Fetch each data from memory using readmem32 twice.
    for (int i = 0; i < (2 * WORD_BYTES); i += WORD_BYTES) {
        data <<= WORD_BITS;
        data |= readmem32(address + WORD_BYTES - i);
    }

    return data;
}

/*
    Takes an address and data 32 bits of data as uint32_t.
    Writes 32 bits (4 bytes) at specified address.
*/
void writemem32(uint32_t address, uint32_t data) {
    // Fetch pointer to the first byte.
    unsigned char *startbyte = fetchbyte(address);

    // Write to memory byte-by-byte.
    for (int i = 0; i < WORD_BYTES; i++) {
        startbyte[i] = data;
        data >>= BYTE_BITS;
    }
}

/*
    Takes an address as uint32_t andi 64 bits of data as uint64_t.
    Writes 64 bits (8 bytes) at specified address.
*/
void writemem64(uint32_t address, uint64_t data) { 
    // Write to memory using writemem32 twice.
    for (int i = 0; i < (2 * WORD_BYTES); i += WORD_BYTES) {
        writemem32(address + i, data);
        data >>= WORD_BITS;
    }
}
