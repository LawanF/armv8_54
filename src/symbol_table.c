/* Implementation for a symbol table.
 * This is a (hash)map from labels (strings, aka char[]) to memory addresses (unsigned integers). */

typedef struct {
    char *label;
    int address;
} MapEntry;
