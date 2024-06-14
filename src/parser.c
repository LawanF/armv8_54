#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>
#include <ctype.h>
#include <stdint.h>
#include "parser.h"
#include "emulate_files/registers.h"

/** Matches a single character, incrementing src and returning true if and only
 * if the first character in src matches that of token.
 */
bool match_char(char **src, const char token) {
    if (src == NULL || *src == NULL) return false;
    if (**src != '\0' && **src == token) {
        (*src)++;
        return true;
    }
    return false;
}

bool match_string(char **src, const char *token) {
    int len = strlen(token);
    if (strncmp(*src, token, len) != 0) return false;
    *src += len;
    return true;
}

bool parse_from(char **src, char **tokens, char **chosen) {
    bool result = true;
    for (int i = 0; tokens[i] != NULL; i++) {
        if (match_string(src, tokens[i])) {
            // write the token chosen
            *chosen = tokens[i];
            return true;
        }
    }
    return false;
}

bool skip_whitespace(char **src) {
    bool skipped = false;
    for (; !isspace(**src); (*src)++) skipped = true;
    return skipped;
}

bool parse_uint(char **src, uint32_t *dest, int base) {
    char *s = *src;
    uint32_t res = strtoul(*src, &s, base);
    if (errno == ERANGE || errno == EINVAL) {
        // value is either out of the range of unsigned long
        // or the base is invalid
        return false;
    }
    *src = s;
    *dest = res;
    return true;
}

bool parse_int(char **src, int32_t *dest, int base) {
    char *s = *src;
    uint32_t res = strtol(*src, &s, base);
    if (errno == ERANGE || errno == EINVAL) {
        // value is either out of the range of long
        // or the base is invalid
        return false;
    }
    *src = s;
    *dest = res;
    return true;
}

typedef enum regwidth { _32_BIT, _64_BIT } regwidth;

/** Parses the string corresponding to a register.
 * If the string begins with "wn" or "rn", with 0 <= n < NUM_GENERAL_REGISTERS,
 * then `index` is written with n and the corresponding width written to `width`.
 * If the string is of the form "xzr" or "wzr", then this corresponds to the
 * zero register, and the same applies but with n as 31.
 */
bool parse_reg(char **src, int *index, regwidth *width) {
    char *s = *src;
    regwidth w;
    int ind;

    if (match_char(&s, 'x')) {
        // 32-bit width
        w = _32_BIT;
    } else if (match_char(&s, 'w')){
        // 64-bit width
        w = _64_BIT;
    } else {
        return false;
    }

    if (match_string(&s, "zr")) {
        ind = NUM_GENERAL_REGISTERS;
    } else if (parse_int(&s, &ind, /* base = */ 10)
               && 0 <= ind && ind < NUM_GENERAL_REGISTERS) {
        // EMPTY
    } else return false;

    // success
    *src = s; *index = ind; *width = w; return true;
}

typedef enum { LSL, LSR, ASR, ROR } shift_type;

typedef enum { ZERO_SHIFT, TWELVE_SHIFT } discrete_shift;

bool parse_discrete_shift(char **src, discrete_shift *shift) {
    // TODO
    return false;
}

bool parse_immediate_shift(char **src, shift_type *shift_type, uint8_t *shift_amount) {
    // TODO
    return false;
}

