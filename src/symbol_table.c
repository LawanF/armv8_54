/* Implementation for a symbol table.
 * This is a (hash)map from labels (strings, aka char[]) to memory addresses (unsigned integers). */

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

/**
 * A structure representing an entry in a bucket.
 * @property label a copy of the label inserted into the bucket
 * @property address the address in memory this label maps to
 */
typedef struct {
    char *label;
    uint32_t address;
} Entry;

/**
 * A structure representing a resizing array of map entries.
 * @property head a pointer to the first of `size` entries in the bucket
 * @property size the current size of the bucket
 */
struct bucket;
typedef struct bucket *Bucket;
struct bucket {
    Entry entry;
    Bucket tail;
};

/**
 * A structure representing a resizing array of buckets.
 * The array attempts to resize when the number of entries exceeds the product of the number of buckets
 * and the load factor (the maximum average number of entries per bucket).
 * The maximum number of buckets is 2^16 (65536), and the number of buckets should always be a power of two
 * for hashing to work properly.
 * @property load_factor the maximum allowed average number of entries per bucket
 * @property buckets a basal pointer to an array of `Bucket`s
 * @property num_buckets the current number of buckets
 */
typedef struct symtable {
    float load_factor;
    Bucket *buckets;
    uint16_t size;
    uint16_t num_buckets;
} *SymbolTable;

/** Creates a symbol table with the given load factor, and one bucket.
 * @param load_factor the load factor of the symbol table (maximum average number of entries per bucket).
 * @returns the symbol table, or `NULL` if the given load factor is invalid (not positive) or creation fails.
 */
SymbolTable symtable_new(float load_factor) {
    if (load_factor <= 0) return NULL;
    // a singleton array of one bucket
    Bucket *buckets = malloc(sizeof(struct bucket) * 1);
    if (buckets == NULL) return NULL;
    buckets[0] = NULL;
    SymbolTable table = malloc(sizeof(struct symtable));
    if (table == NULL) {
        free(buckets);
        return NULL;
    }
    table->load_factor = load_factor;
    table->buckets     = buckets;
    table->size        = 0;
    table->num_buckets = 1;
    return table;
}
