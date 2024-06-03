/* Implementation for a symbol table.
 * This is a (hash)map from labels (strings, aka char[]) to memory addresses (unsigned integers). */

#include <stdint.h>

typedef struct {
    char *label;
    uint32_t address;
} MapEntry;

typedef struct {
    MapEntry *entries;
    uint16_t size;
    uint16_t capacity;
} MapBucket;
