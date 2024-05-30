#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "memory.h"

// Define a char[] representing the machine memory.
// Data is stored in little-endian.
static unsigned char memory[MEMORY_SIZE];

/*
    Clears memory, setting all values to 0.
*/
void initmem() {
    memset(memory, 0, MEMORY_SIZE * sizeof(char));
}

/*
    Takes a 21-bit address as uint32_t.
    Checks if address is less than or equal to (MEMORY_SIZE - 4) and
    if address adheres to 4-byte boundary (for readmem32 and writemem32).
*/
static void checkaddress32(uint32_t address) {
    // Check if address is a multiple of 4.
    if ((address % 4) != 0) {
        fprintf(stderr, "checkaddress32: address %08x is not multiple of 4\n", address);
        exit(1);
    }

    // Check if address is within bounds of memory.
    if (address > MEMORY_SIZE - 4) {
        fprintf(stderr, "checkaddress32: address %08x is out of bounds\n", address);
        exit(1);
    }
}

/*
    Takes a 21-bit address as uint32_t.
    Returns a pointer to the byte at that address as unsigned char.
*/
static unsigned char *fetchbyte(uint32_t address) {
    // Check address.
    checkaddress32(address);

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
    for (int i = 0; i < 4; i++) {
        data |= startbyte[i] << (8 * i);
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

    // Fetch each data from memory using readmem32.
    for (int i = 0; i < 8; i += 4) {
        data <<= 32;
        data |= readmem32(address + 4 - i);
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
    for (int i = 0; i < 4; i++) {
        startbyte[i] = data;
        data >>= 8;
    }
}

/*
    Takes an address as uint32_t andi 64 bits of data as uint64_t.
    Writes 64 bits (8 bytes) at specified address.
*/
void writemem64(uint32_t address, uint64_t data) { 
    // Write to memory using writemem32.
    for (int i = 0; i < 8; i += 4) {
        writemem32(address + i, data);
        data >>= 32;
    }
}

int main() {
    writemem64(MEMORY_SIZE - 4, 0xff);
    return 0;
}
