/* Implementation for a symbol table.
 * This is a (hash)map from labels (strings, aka char[]) to memory addresses (unsigned integers). */

#include <assert.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

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

/** Frees the contents of a linked list with a given head node, including the memory used to store it.
 * @param head the head of the list to be freed
 */
static void bucket_free_all(Bucket head) {
    Bucket next_head;
    for (Bucket bucket = head; bucket != NULL; bucket = next_head) {
        free(bucket->entry.label);
        next_head = bucket->tail;
        free(bucket);
    }
    free(head);
}

/** Prepends an element to the given bucket, modifying it in place.
 * @param bucket the bucket to be appended
 * @returns `true` if the bucket was modified, `false` if addition fails (i.e. if memory allocation fails)
 */
static bool bucket_add(Bucket bucket, Entry entry) {
    Bucket prepended = malloc(sizeof(struct bucket));
    if (prepended == NULL) return false;
    *prepended = (struct bucket) { .entry = entry, .tail = bucket };
    bucket = prepended;
    return true;
}

/** Uses linear search to determine whether an entry with a given key is in the bucket.
 * A precondition is that the key is not `NULL`.
 * @param bucket the bucket to be searched through
 * @returns `true` if and only if an entry is in the bucket.
 */
static bool bucket_contains(Bucket bucket, char *key) {
    for (Bucket b = bucket; b != NULL; b = b->tail) {
        if (strcmp(b->entry.label, key) == 0) return true;
    }
    return false;
}

/** Removes (and frees) the first element with a given key in the symbol table.
 * The reference to the bucket given in `head_ptr` will be written with the head of the new list,
 * and `dest` will be written with the value associated with the `key` (if it exists).
 * @param head_ptr the given symbol table
 * @param key the string to search for in the bucket
 * @param dest a pointer to which the associated value of `key` is written, if it exists.
 * @returns `true` if the element was removed, and `false` if the bucket is unmodified.
 */
static bool bucket_remove(Bucket *head_ptr, char *key, uint16_t *dest) {
    Bucket par = NULL;
    for (Bucket cur = *head_ptr; cur != NULL; cur = cur->tail) {
        if (strcmp(cur->entry.label, key) == 0) {
            // match found
            if (par == NULL) {
                // change the head node
                *head_ptr = cur->tail;
            } else {
                // unlink the current entry from the list
                par->tail = cur->tail;
            }
            *dest = cur->entry.address;
            // destroy the entry
            free(cur->entry.label);
            free(cur);
            return true;
        }
        par = cur;
    }
    return false;
}

/** Returns a pointer to an array of `symtable->size` entries in the symbol table
 * @param symtable the given symbol table
 * @returns an unsorted copy of the entries in the symtable, or `NULL` if memory allocation failed
 */
static Entry *symtable_entries(SymbolTable symtable) {
    Entry *entries = malloc(sizeof(Entry) * symtable->size);
    if (entries == NULL) return NULL;
    uint16_t i = 0;
    for (uint16_t j = 0; j < symtable->num_buckets; j++) {
        for (Bucket b = symtable->buckets[j]; b != NULL; b = b->tail) {
            entries[i++] = b->entry;
        }
    }
    assert(i == symtable->size);
    return entries;
}

/** Frees the buckets inside the symbol table.
 * @param symtable the given symbol table
 */
static void symtable_free_buckets(SymbolTable symtable) {
    for (uint16_t i = 0; i < symtable->num_buckets; i++) {
        bucket_free_all(symtable->buckets[i]);
    }
    free(symtable->buckets);
}

/** A hashing function used to index the symtable using the djb2 algorithm in the given link.
 * @param str the string to be hashed
 * @returns the hashed string, 16 bits in length
 * @see http://www.cse.yorku.ca/~oz/hash.html
 */
static uint16_t string_hash(const char *str) {
    uint16_t hash = 5381;
    for (int c; (c = *str++); hash = ((hash << 5) + hash) + c); // hash = hash * 33 + c
    return hash;
}

/** Returns the index in which a given key is stored in the symbol table.
 * A precondition is that the number of buckets in the table is a power of two.
 * @param symtable the symbol table to be indexed
 * @param key the string to be hashed
 */
static uint16_t symtable_bucket_index(SymbolTable symtable, const char *key) {
    return string_hash(key) & (symtable->num_buckets - 1);
}

