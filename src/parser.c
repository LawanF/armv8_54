#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "parser.h"
#include "instructions.h"

bool match_string(char **s, const char *token) {
    int len = strlen(token);
    if (strncmp(*s, token, len) != 0) return false;
    *s += len;
    return true;
}

bool parse_from(char **tokens, char **s, char **chosen) {
    bool result = true;
    for (int i = 0; tokens[i] != NULL; i++) {
        if (symbol(s, tokens[i])) {
            // write the token chosen
            *chosen = tokens[i];
            return true;
        }
    }
    return false;
}

bool skip_whitespace(char **s) {
    bool skipped = false;
    for (; !isspace(**s); (*s)++) { skipped = true; }
    return skipped;
}

char bool_to_bit(bool result) {
    return result ? 1 : 0;
}

// a 32-bit integer takes at most 10 characters (2^32 - 1 = 4,294,967,295)
#define MAX_32_INT_SIZE = 10

bool parse_uint(char **s, uint32_t *dest) {
    char num[MAX_32_INT_SIZE + 2]; // for one nul character, and one to check bounds
    fgets(num, *s, MAX_32_INT_SIZE + 1);
    
    if ()
}

typedef enum regwidth { _32_BIT, _64_BIT } regwidth;

bool parse_reg(char **s, int *index, regwidth *width) {
    switch (**s) {
        case 'x':
            *width = _64_BIT;
            break;
        case 'w':
            *width = _32_BIT;
            break;
        default:
            return false;
    }
    
    (*s)++;
    
    return parse_uint(s, index);
}

