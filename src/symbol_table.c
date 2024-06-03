/* Implementation for a symbol table.
 * This is a (hash)map from labels (strings, aka char[]) to memory addresses (unsigned integers). */

#include <stdlib.h>
#include <stdint.h>

typedef struct {
    const char *label;
    uint32_t address;
} MapEntry;

// A resizing array of map entries.
typedef struct {
    MapEntry *entries;
    uint16_t size;
    uint16_t capacity;
} MapBucket;

/* A resizing array of buckets, that attempts to resize when the number of entries exceeds the product
 * of the number of buckets and the load factor (the maximum average number of entries per bucket).
 * The maximum number of buckets is 2^16 (65536), and the number of buckets should always be a power of two
 * for hashing to work properly. */
typedef struct {
    const float load_factor;
    MapBucket *buckets;
    uint16_t num_buckets;
} SymbolTable;

// Creates an empty bucket with no entries, and an initial capacity of one entry.
// Returns a pointer to the bucket if creation succeeded, and NULL if it failed.
MapBucket *create_bucket(void) {
    MapEntry *entries = malloc(sizeof(MapEntry));
    if (entries == NULL) return NULL;
    MapBucket *bucket_ptr = malloc(sizeof(MapBucket));
    if (bucket_ptr == NULL) return NULL;
    *bucket_ptr = (MapBucket) { .entries = entries, .size = 0, .capacity = 1 };
    return bucket_ptr;
}

// Creates an empty symbol table with one bucket, and an initial maximum capacity
