/* Implementation for a symbol table.
 * This is a (hash)map from labels (strings, aka char[]) to memory addresses (unsigned integers). */

#include <cmath>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#define MAX_NUM_BUCKETS (0x1UL << 15)

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
 * A structure representing a linked list of map entries.
 * @property entry a pointer to the current of `size` entries in the bucket
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
    SymbolTable symtable = malloc(sizeof(struct symtable));
    if (symtable == NULL) {
        free(buckets);
        return NULL;
    }
    symtable->load_factor = load_factor;
    symtable->buckets     = buckets;
    symtable->size        = 0;
    symtable->num_buckets = 1;
    return symtable;
}

static void bucket_free(Bucket bucket) {
    Bucket next_bucket;
    for (Bucket cur_bucket = bucket; cur_bucket != NULL; cur_bucket = next_bucket) {
        free(cur_bucket->entry.label);
        next_bucket = cur_bucket->tail;
        free(cur_bucket);
    }
}

/** Prepends an element to the given bucket, modifying it in place.
 * @param bucket the bucket to be appended
 * @returns 1 if the bucket was modified, 0 if addition fails (i.e. if memory allocation fails)
 */
static int bucket_add(Bucket *bucket, Entry entry) {
    Bucket prepended = malloc(sizeof(struct bucket));
    if (prepended == NULL) return 0;
    *prepended = (struct bucket) { .entry = entry, .tail = *bucket };
    *bucket = prepended;
    return 1;
}

/** Frees the buckets inside the symbol table.
 * @param symtable the given symbol table
 */
static void symtable_free_buckets(SymbolTable symtable) {
    for (uint16_t i = 0; i < symtable->num_buckets; i++) {
        bucket_free(symtable->buckets[i]);
    }
    free(symtable->buckets);
}
