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

/** Creates a symbol table with a given number of buckets.
 * A precondition is that the number of buckets is a power of two.
 * @param load_factor the load factor of the symbol table
 * @param num_buckets the number of buckets in the symbol table
 * @returns the symbol table, or `NULL` if creation fails
 * @see symtable_new
 */
static SymbolTable symtable_num_buckets(float load_factor, uint16_t num_buckets) {
    if (load_factor <= 0) return NULL;
    // count the number of ones; it is one if and only if num_buckets is a power of two
    uint8_t num_ones = 0;
    for (int i = 0; i < 16; i++) {
        num_ones += (num_buckets >> i) & 0x1;
    }
    if (num_ones != 1) return NULL;
    // a singleton array of one bucket
    Bucket *buckets = malloc(sizeof(struct bucket) * num_buckets);
    if (buckets == NULL) return NULL;
    // initialise the buckets with NULL
    for (int i = 0; i < num_buckets; i++) {
        buckets[i] = NULL;
    }
    SymbolTable symtable = malloc(sizeof(struct symtable));
    if (symtable == NULL) {
        free(buckets);
        return NULL;
    }
    symtable->load_factor = load_factor;
    symtable->buckets     = buckets;
    symtable->size        = 0;
    symtable->num_buckets = num_buckets;
    return symtable;
}

/** Creates a symbol table with the given load factor, and one bucket.
 * @param load_factor the load factor of the symbol table (maximum average number of entries per bucket).
 * @returns the symbol table, or `NULL` if the given load factor is invalid (not positive) or creation fails.
 */
SymbolTable symtable_new(float load_factor) {
    return symtable_num_buckets(load_factor, /* num_buckets = */ 1);
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
 * @param bucket_ptr a pointer to the bucket to be appended
 * @returns `true` if the bucket was modified, `false` if addition fails (i.e. if memory allocation fails)
 */
static bool bucket_add(Bucket *head_ptr, Entry entry) {
    Bucket prepended = malloc(sizeof(struct bucket));
    if (prepended == NULL) return false;
    *prepended = (struct bucket) { .entry = entry, .tail = *head_ptr };
    *head_ptr = prepended;
    return true;
}

/** Uses linear search to determine whether an entry with a given key is in the bucket.
 * A precondition is that the key is not `NULL`.
 * @param bucket the bucket to be searched through
 * @returns `true` if and only if an entry is in the bucket.
 */
static bool bucket_contains(Bucket bucket, const char *key) {
    for (Bucket b = bucket; b != NULL; b = b->tail) {
        if (strcmp(b->entry.label, key) == 0) return true;
    }
    return false;
}

/** Performs linear search to find an entry associated with a given key in a bucket.
 * If `dest` is not `NULL`, `dest` is written with the value if such an entry is found.
 * @param bucket the bucket to be searched through
 * @param key the label to be searched for
 * @param dest a pointer to which to write the associated address, if it is found
 * @returns `true` if and only if an entry with the given key exists in the bucket.
 */
static bool bucket_find(Bucket bucket, const char *key, uint16_t *dest) {
    for (Bucket b = bucket; b != NULL; b = b->tail) {
        if (strcmp(b->entry.label, key) == 0) {
            if (dest != NULL) *dest = b->entry.address;
            return true;
        }
    }
    return false;
}

/** Removes (and frees) the first element with a given key in the symbol table.
 * The reference to the bucket given in `head_ptr` will be written with the head of the new list,
 * and if `dest` is not NULL, it will be written with the value associated with the `key` (if it exists).
 * @param head_ptr a pointer to the bucket to be modified
 * @param key the string to search for in the bucket
 * @param dest a pointer to which the previous associated value of `key` is written, if it exists.
 * @returns `true` if the element was removed, and `false` if the bucket is unmodified.
 */
static bool bucket_remove(Bucket *head_ptr, const char *key, uint16_t *dest) {
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
            if (dest != NULL) *dest = cur->entry.address;
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

/** Frees the contents of the symbol table.
 * @param symtable the given symbol table to be unallocated from memory
 */
void symtable_free(SymbolTable symtable) {
    symtable_free_buckets(symtable);
    free(symtable);
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

/** Determines whether an entry under a given key exists in a symbol table
 * @param symtable the symbol table to be searched
 * @param key the symbol to be searched for in the symbol table
 * @returns `true` if a value is associated to the key in the map, and `false` otherwise
 */
bool symtable_contains(SymbolTable symtable, const char *key) {
    uint16_t bucket_index = symtable_bucket_index(symtable, key);
    return bucket_contains(symtable->buckets[bucket_index], key);
}

/** Searches for an entry with the given label `key` in the symbol table.
 * If `dest` is not `NULL`, `dest` is written with the address if such an entry is found.
 * @param symtable the symbol table to be searched through
 * @param key the symbol to be searched for
 * @param dest a pointer to which to write the associated address, if it is found
 * @returns `true` if and only if an entry with the given key exists in the symbol table.
 */
static bool symtable_find(SymbolTable symtable, const char *key, uint16_t *dest) {
    uint16_t bucket_index = symtable_bucket_index(symtable, key);
    return bucket_find(symtable->buckets[bucket_index], key, dest);
}

/** Removes the entry with a given key in the symbol table, writing the previous associated
 * address to `dest`.
 * @param head_ptr the given symbol table
 * @param key the string to search for in the bucket
 * @param dest a pointer to which the previous associated address under `key` is written, if it exists.
 * @returns `true` if the entry was removed, and `false` if the symbol table was unmodified.
 */
bool symtable_remove(SymbolTable symtable, const char *key, uint16_t *dest) {
    uint16_t bucket_index = symtable_bucket_index(symtable, key);
    Bucket *head_ptr = &symtable->buckets[bucket_index];
    if (bucket_remove(head_ptr, key, dest)) {
        symtable->size--;
        return true;
    }
    return false;
}

extern bool symtable_set(SymbolTable symtable, const char *key, const uint16_t address);

/** Resizes the symbol table to contain twice the number of buckets.
 * @param symtable the symbol table to resize
 * @returns `true` if resizing has succeeded (i.e. memory allocation has succeeded), and `false` otherwise.
 */
static bool symtable_resize(SymbolTable symtable) {
    if (symtable->num_buckets >= MAX_NUM_BUCKETS / 2) return false;
    SymbolTable new_table = symtable_num_buckets(symtable->load_factor, symtable->num_buckets * 2);
    if (new_table == NULL) return false;
    Entry *entries = symtable_entries(symtable);
    if (entries == NULL) {
        symtable_free(new_table);
        return false;
    }
    bool success = true;
    for (int i = 0; i < symtable->size; i++) {
        Entry e = entries[i];
        success = success && symtable_set(new_table, e.label, e.address);
    }
    free(entries);
    if (!success) {
        // destroy the temporary table
        symtable_free(symtable);
        return false;
    } else {
        // clear the buckets in the old symbol table and copy the new ones over
        symtable_free_buckets(symtable);
        memcpy(symtable, new_table, sizeof(struct symtable));
        return true;
    }
}

/** Adds a given entry with key and associated address to the symbol table.
 * @param symtable the symbol table to add an entry added to
 * @param key the label to be associated
 * @param address the location in memory of the label
 * @returns `true` if addition succeeded, and `false` otherwise
 */
bool symtable_set(SymbolTable symtable, const char *key, const uint16_t address) {
    uint16_t bucket_index = symtable_bucket_index(symtable, key);
    Bucket *head_ptr = &symtable->buckets[bucket_index];
    if (!bucket_contains(*head_ptr, key)) {
        Entry *entry_ptr = malloc(sizeof(Entry));
        if (entry_ptr == NULL) return false;
        char *str = malloc(strlen(key));
        if (str == NULL) {
            free(entry_ptr);
            return false;
        }
        strcpy(str, key);
        *entry_ptr = (Entry) { .label = str, .address = address };
        if (!bucket_add(head_ptr, *entry_ptr)) {
            free(str);
            free(entry_ptr);
            return false;
        }
        symtable->size++;
        return true;
    } else {
        // an entry already exists in the symbol table, remove this from the symbol table
        return symtable_remove(symtable, key, NULL) && symtable_set(symtable, key, address);
    }
}
