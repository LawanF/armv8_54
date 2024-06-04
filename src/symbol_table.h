/** A module for creating symbol tables (hash maps from labels (of type char *) to addresses (of type uint16_t)).
 * The hash map is dynamically allocated.
 */

#include <stdint.h>
#include <stdbool.h>

typedef struct symtable *SymbolTable;

SymbolTable symtable_new(float load_factor);

void symtable_free(SymbolTable symtable);

bool symtable_contains(SymbolTable symtable, const char *key);

bool symtable_find(SymbolTable symtable, const char *key, uint32_t *dest);

bool symtable_remove(SymbolTable symtable, const char *key, uint32_t *dest);

bool symtable_set(SymbolTable symtable, const char *key, const uint32_t address);
