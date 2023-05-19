#include "math_bigfloat.h"

#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

bigfloat bf_from_f32(mg_arena* arena, f32 num, u32 prec);
bigfloat bf_from_f64(mg_arena* arena, f64 num, u32 prec);

#define HEXCHAR_VAL(c) ((c) <= '9' ? (c) - '0' : tolower(c) - 'a' + 10)
static bigfloat _bf_from_hex_str(mg_arena* arena, string8 str, u32 prec, b32 negative);
bigfloat bf_from_str(mg_arena* arena, string8 str, u32 base, u32 prec) {
    if (str.size == 0) {
        fprintf(stderr, "Invalid string for bigfloat\n");
        return (bigfloat){ 0 };
    }

    b32 negative = str.str[0] == '-';
    if (negative) {
        str.str += 1;
        str.size--;
    }

    // Strip leading and trailing zeros
    u64 post_decimal_size = 0;
    u64 decimal_index = 0;
    if (str8_index_of(str, (u8)'.', &decimal_index)) {
        post_decimal_size = str.size - decimal_index - 1;
    }
    while (str.size > 2 + post_decimal_size && str.str[0] == '0') {
        str.str += 1;
        str.size--;
    }
    if (post_decimal_size) {
        u64 init_pre_size = str.size - post_decimal_size;
        while (str.size > (1 + init_pre_size) && str.str[str.size - 1] == '0') {
            str.size--;
        }
    }

    if (str8_equals(str, STR8("0")) || str8_equals(str, STR8(".0")) || str8_equals(str, STR8("0.0"))) {
        bigfloat out = {
            .prec = prec,
            .size = 1 * (negative ? -1 : 1),
            .exp = 0,
            .limbs = MGA_PUSH_ZERO_ARRAY(arena, u32, prec)
        };
        
        return out;
    }

    if (base == 16)
        return _bf_from_hex_str(arena, str, prec, negative);

    fprintf(stderr, "Unsuporrted base %u\n", base);
    return (bigfloat) { 0 };
}
static bigfloat _bf_from_hex_str(mg_arena* arena, string8 str, u32 prec, b32 negative) {
    u64 decimal_index = str.size;
    str8_index_of(str, (u8)'.', &decimal_index);

    mga_temp scratch = mga_scratch_get(&arena, 1);

    u32 init_limbs_size = (decimal_index + 7) / 8 + (str.size - decimal_index + 7) / 8;
    u32 init_decimal_limb = (str.size - decimal_index + 7) / 8; // Index of first decimal limb
    u32* init_limbs = MGA_PUSH_ZERO_ARRAY(scratch.arena, u32, init_limbs_size);

    // Parsing decimal portion of number
    u32 shift = 28;
    for (u64 i = decimal_index + 1; i < str.size; i++) {
        u32 char_val = HEXCHAR_VAL(str.str[i]);
        i64 limb_index = (i64)init_decimal_limb - 1 - (i - decimal_index - 1) / 8;
        init_limbs[limb_index] |= char_val << shift;

        if (shift == 0) shift = 32;
        shift -= 4;
    }

    // Parsing integer portion of number
    shift = 0;
    for (i64 i = decimal_index - 1; i >= 0; i--) {
        u32 char_val = HEXCHAR_VAL(str.str[i]);
        u32 limb_index = ((decimal_index - 1 - i) / 8) + init_decimal_limb;
        init_limbs[limb_index] |= char_val << shift;
        shift = (shift + 4) & 31;
    }

    u32 most_sig_digit = init_limbs_size - 1;
    for (i64 i = init_limbs_size - 1; i >= 0; i--) {
        if (init_limbs[i] != 0) {
            most_sig_digit = i;
            break;
        }
    }
    u32 least_sig_digit = 0;
    for (u32 i = 0; i < init_limbs_size; i++) {
        if (init_limbs[i] != 0) {
            least_sig_digit = i;
            break;
        }
    }

    u32 abs_size = MIN(most_sig_digit + 1, prec) - least_sig_digit;

    bigfloat out = {
        .prec = prec,
        .size = abs_size * (negative ? -1 : 1),
        .exp = (i32)most_sig_digit - (i32)init_decimal_limb,
        .limbs = MGA_PUSH_ZERO_ARRAY(arena, u32, prec)
    };

    for (u32 i = 0; i < abs_size; i++) {
        out.limbs[i] = init_limbs[i + least_sig_digit];
    }
    
    mga_scratch_release(scratch);

    return out;
}

b32 bf_add_ip(bigfloat* out, const bigfloat* a, const bigfloat* b) {
    if ((a->size ^ b->size) < 0) {
        // a and b have different signs

        // TODO: call sub functions
    }

    if (bf_cmp(a, b) < 0) {
        const bigfloat* temp = a;
        a = b;
        b = temp;
    }

    u32 abs_asize = ABS(a->size);
    u32 abs_bsize = ABS(b->size);

    i64 min_exp_digit = MIN((i64)a->exp - abs_asize, (i64)b->exp - abs_bsize);
    i64 max_exp_digit = a->exp;

    u32 max_prec = (u32)(max_exp_digit - min_exp_digit);

    printf("min: %ld, max: %ld, prec: %u\n", min_exp_digit, max_exp_digit, max_prec);

    // goal: loop through from 0 to out->prec and add numbers
    // min = MIN(exp - ABS(size))
    // max = MAX(exp) (should be a->exp)
    // Loop through as many digits as possible in the range and add

    return false;
}
b32 bf_sub_ip(bigfloat* out, const bigfloat* a, const bigfloat* b);
b32 bf_mul_ip(bigfloat* out, const bigfloat* a, const bigfloat* b);
b32 bf_div_ip(bigfloat* q, const bigfloat* a, const bigfloat* b, bigfloat* r);

bigfloat bf_add(mg_arena* arena, const bigfloat* a, const bigfloat* b);
bigfloat bf_sub(mg_arena* arena, const bigfloat* a, const bigfloat* b);
bigfloat bf_mul(mg_arena* arena, const bigfloat* a, const bigfloat* b);
bigfloat bf_div(mg_arena* arena, const bigfloat* a, const bigfloat* b, bigfloat* r);

b32 bf_is_zero(const bigfloat* bf);
b32 bf_equals(const bigfloat* a, const bigfloat* b);
i32 bf_cmp(const bigfloat* a, const bigfloat* b) {
    if ((a->size ^ b->size) < 0) {
        return a->size > b->size ? 1 : -1;
    }
    i32 sign = SIGN(a->size);

    if (a->exp != b->exp) {
        return (a->exp > b->exp ? 1 : -1) * sign;
    }

    i64 ai = ABS(a->size) - 1;
    i64 bi = ABS(b->size) - 1;

     if (ai != bi) {
         return (ai > bi ? 1 : -1) * sign;
     }
    
    for (; ai >= 0 && bi >= 0; ai--, bi--) {
        if (a->limbs[ai] != b->limbs[bi]) {
            return (a->limbs[ai] > b->limbs[bi] ? 1 : -1) * sign;
        }
    }

    return 0;
}

string8 bf_to_str(mg_arena* arena, const bigfloat* bf, u32 base);
string8 bf_print(const bigfloat* bf, u32 base);
