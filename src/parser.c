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

/** Matches a string of nul-terminated characters, incrementing src and
 * returning true if and only if the beginning of src matches all of token.
 */
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
    int32_t res = strtol(*src, &s, base);
    if (errno == ERANGE || errno == EINVAL) {
        // value is either out of the range of long
        // or the base is invalid
        return false;
    }
    *src = s;
    *dest = res;
    return true;
}

/** Parses an immediate value: a string of the form "#imm", where
 * imm is an unsigned integer, setting `dest` to be this value if
 * parsing succeeds.
 * @returns true if and only if parsing succeeds
 */
bool parse_immediate(char **src, uint32_t *dest) {
    char *s = *src;
    uint32_t val;
    if (!(match_char(&s, '#')
          && parse_uint(&s, &val, /* base = */ 10))) {
        return false;
    }
    *src = s;
    *dest = val;
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

/** Parses a discrete (left) shift, a string of the form ", lsl #0"
  * or ", lsl #12".
  * @returns `true` (and writes to `discrete_shift`) if parsing succeeds,
  * and `false` otherwise
  */
bool parse_discrete_shift(char **src, discrete_shift *shift) {
    char *s = *src;
    unsigned int shift_amount;
    bool success = match_char(&s, ',')
                && skip_whitespace(&s)
                && match_string(&s, "lsl")
                && skip_whitespace(&s)
                && parse_immediate(&s, &shift_amount)
                && (shift_amount == 0 || shift_amount == 12);
    if (!success) return false;

    *src = s;
    *shift = shift_amount ? TWELVE_SHIFT : ZERO_SHIFT;
    return true;
}

// Parses a shift type, a string that is one of ["lsl", "lsr", "asr", "ror"].
static bool parse_shift_type(char **src, shift_type *dest_type) {
    for (int i = 0; i < ARRAY_LEN(shift_types); i++) {
        if (match_string(src, shift_types[i])) {
            *dest_type = i;
            return true;
        }
    }
    return false;
}

/** Parses an immediate shift, a string of the form ", [shift] #[imm]", where
 * [shift] is one of ["lsl", "lsr", "asr", "ror"], and [imm] is an integer
 * from 0 to 63. Additional validation will be required to ensure the value is
 * between 0 and 31 for the 32-bit variant of instructions.
 * @returns `true` (and writes fields) if parsing succeeds,
 * and `false` otherwise
 */
bool parse_immediate_shift(char **src, shift_type *dest_type, uint8_t *dest_amount) {
    char *s = *src;
    shift_type type;
    uint32_t amount;
    bool success = match_char(&s, ',')
                && skip_whitespace(&s)
                && parse_shift_type(&s, &type)
                && skip_whitespace(&s)
                && parse_immediate(&s, &amount)
                && (0 <= amount && amount < 64);
    if (!success) return false;

    *src = s;
    *dest_type = type;
    *dest_amount = amount;
    return true;
}

