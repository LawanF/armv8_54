/** A module for creating symbol tables (hash maps from labels (of type char *) to addresses (of type uint32_t)).
 * The hash map is dynamically allocated.
 */

#include <stdint.h>
#include <stdbool.h>

typedef struct symtable *SymbolTable;

SymbolTable symtable_new(float load_factor);

void symtable_free(SymbolTable symtable);

bool symtable_contains(SymbolTable symtable, const char *key);

bool symtable_get(SymbolTable symtable, const char *key, uint32_t *dest);

bool single_symtable_remove(SymbolTable symtable, const char *key, uint32_t *dest);

bool single_symtable_set(SymbolTable symtable, const char *key, const uint32_t address);

bool multi_symtable_add(SymbolTable symtable, const char *key, const uint32_t address);

bool multi_symtable_remove_last(SymbolTable symtable, const char *key, uint32_t *dest);

bool multi_symtable_remove_all(SymbolTable symtable, const char *key, uint32_t *dest);
