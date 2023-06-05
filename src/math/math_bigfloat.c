#include "math_bigfloat.h"

#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

static void _bf_fix_leading_zeros(bigfloat* bf);
static void _bf_fix_leading_zeros_no_exp(bigfloat* bf);

bigfloat bf_create(mg_arena* arena, u32 prec) {
    return (bigfloat){
        .prec = prec,
        .size = 1,
        .exp = 0,
        .limbs = MGA_PUSH_ZERO_ARRAY(arena, u32, prec)
    };
}
bigfloat bf_copy(mg_arena* arena, bigfloat* bf) {
    bigfloat out = {
        .prec = bf->prec,
        .size = bf->size,
        .exp = bf->exp,
        .limbs = MGA_PUSH_ARRAY(arena, u32, bf->prec)
    };

    memcpy(out.limbs, bf->limbs, sizeof(u32) * bf->prec);

    return out;
}
bigfloat bf_from_f64(mg_arena* arena, f64 num, u32 prec) {
    bigfloat out = bf_create(arena, prec);
    bf_set_f64(&out, num);
    return out;
}
bigfloat bf_from_i64(mg_arena* arena, i64 num, u32 prec) {
    bigfloat out = bf_create(arena, prec);
    bf_set_i64(&out, num);
    return out;
}

void bf_set(bigfloat* a, const bigfloat* b) {
    u32 abs_size = ABS(b->size);
    u32 asize = MIN(abs_size, a->prec);
    
    a->exp = b->exp;
    a->size = asize * SIGN(b->size);

    u32 offset = (a->prec < abs_size) ? abs_size - a->prec : 0;
    for (u32 i = 0; i < asize; i++) {
        a->limbs[i] = b->limbs[i + offset];
    }
}

typedef struct {
    u16 sign, exponent;
    u64 mantissa;
} float_parts;
typedef union {
    f64 num; u64 bits;
} f64_bits;

float_parts f64_get_parts(f64 num) {
    f64_bits bits = { .num = num };
    float_parts out = { 0 };

    out.sign = bits.bits >> 63;
    out.exponent = (bits.bits >> 52) & 0x7ff;
    out.mantissa = bits.bits & 0xfffffffffffff;

    return out;
}
// TODO: Handle denormalized numbers
// Based on gmp extract-dbl.c
void bf_set_f64(bigfloat* bf, f64 num) {
    if (bf->prec < 3) {
        // I do not want to deal with annoying cases
        fprintf(stderr, "Must have 3 limbs of precision for doubles\n");
        return;
    }

    if (isnan(num) || isinf(num)) {
        fprintf(stderr, "Cannot set bigfloat to nan or inf\n");
        return;
    }
    if (num == 0.0) {
        bf_set_zero(bf);
        return;
    }
    
    float_parts parts = f64_get_parts(num);
    
    i32 binary_exp = (i32)parts.exponent - 1023 + 1; // For the leading 1
    u64 man = ((u64)1 << 63) | (parts.mantissa << 11);

    u32 shift = (u32)(binary_exp + 64 * 32) % 32;
    i32 exp = (binary_exp + 64 * 32) / 32 - 64;

    if (shift != 0) {
        bf->limbs[2] = (u32)(man >> (64 - shift));
        bf->limbs[1] = (u32)((man >> 32) << shift) | ((man & 0xffffffff) >> (32 - shift));
        bf->limbs[0] = (u32)((man & 0xffffffff) << shift);
    } else {
        bf->limbs[2] = (u32)(man >> 32);
        bf->limbs[1] = (u32)(man & 0xffffffff);
        bf->limbs[0] = 0;

        exp--;
    }

    u32 size = 3;
    u32 least_sig_digit = 0;
    for (u32 i = 0; i < bf->prec; i++) {
        if (bf->limbs[i] != 0) {
            least_sig_digit = i;
            break;
        }
    }

    if (least_sig_digit != 0) {
        //bf->exp += least_sig_digit;

        size -= least_sig_digit;
        for (u32 i = 0; i < size; i++) {
            bf->limbs[i] = bf->limbs[i + least_sig_digit];
        }
    }

    bf->size = parts.sign ? (-(i32)size) : (i32)size;
    bf->exp = exp;

}
void bf_set_i64(bigfloat* bf, i64 num) {
    u32 abs_size = MIN(bf->prec, 2);
    i32 sign = SIGN(num);

    bf->size = (i32)abs_size * sign;
    bf->exp = 1;
    num *= sign;

    u32 temp_limbs[2] = { num & BIGFLOAT_MASK, num >> 32 };
    for (i32 i = abs_size; i >= 0; i--) {
        bf->limbs[i] = temp_limbs[i];
    }
    
    _bf_fix_leading_zeros(bf);

}
void bf_set_zero(bigfloat* bf) {
    bf->exp = 0;
    bf->size = 1;

    memset(bf->limbs, 0, sizeof(u32) * bf->prec);
}

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

    fprintf(stderr, "Unsupported base %u for bf_from_str\n", base);
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

    bigfloat out = (bigfloat){
        .prec = prec,
        .size = abs_size * (negative ? -1 : 1),
        .exp = (i32)most_sig_digit - (i32)init_decimal_limb,
        .limbs = MGA_PUSH_ZERO_ARRAY(arena, u32, prec)
    };

    u32 offset = least_sig_digit;
    if (prec < most_sig_digit + 1) {
        offset += most_sig_digit + 1 - prec;
    }
    for (u32 i = 0; i < abs_size; i++) {
        out.limbs[i] = init_limbs[i + offset];
    }
    
    mga_scratch_release(scratch);

    _bf_fix_leading_zeros(&out);
    
    return out;
}

void bf_add(bigfloat* out, const bigfloat* a, const bigfloat* b) {
    if (bf_is_zero(a)) {
        bf_set(out, b);
        return;
    }
    if (bf_is_zero(b)) {
        bf_set(out, a);
        return;
    }

    if ((a->size ^ b->size) < 0) {
        // a + (-b) = a - b
        // -a + b = -a - (-b)

        bigfloat neg_b = {
            .prec = b->prec,
            .size = -b->size,
            .exp = b->exp,
            .limbs = b->limbs
        };

        bf_sub(out, a, &neg_b);
        
        return;
    }

    if (a->exp < b->exp) {
        const bigfloat* temp = a;
        a = b;
        b = temp;
    }

    u32 asize = ABS(a->size);
    u32 bsize = ABS(b->size);
    i32 real_bsize = bsize;
    u32* alimbs = a->limbs;
    u32* blimbs = b->limbs;
    i32 exp_diff = a->exp - b->exp;
    i32 out_size = 0;
    i32 prec = out->prec - 1; // the -1 prevents a memory bug during the final carry

    // Remove parts of a and b that go beyond out->prec
    if ((i32)asize > prec) {
        alimbs += asize - prec;
        asize = (u32)prec;
    }
    // b is somewhere to the left of a
    // exp_diff compensates for this
    if ((i64)bsize + exp_diff > (i64)prec) {
        blimbs += bsize + exp_diff - prec;
        real_bsize = (i32)prec - exp_diff;
        bsize = (u32)MAX(0, real_bsize);
    }

    mga_temp scratch = mga_scratch_get(NULL, 0);
    if (out == a) {
        u32* tmp_limbs = alimbs;
        alimbs = MGA_PUSH_ARRAY(scratch.arena, u32, asize);
        memcpy(alimbs, tmp_limbs, sizeof(u32) * asize);
    }
    if (out == b) {
        u32* tmp_limbs = blimbs;
        blimbs = MGA_PUSH_ARRAY(scratch.arena, u32, bsize);
        memcpy(blimbs, tmp_limbs, sizeof(u32) * bsize);
    }

    if ((i64)asize > exp_diff) {
        // a and b overlap

        u32* add_out = NULL;

        if (bsize + exp_diff <= asize) {
            // a extends beyond b
            // aaaaa
            //   bb

            out_size = asize;

            u32 copy_size = asize - exp_diff - bsize;
            memcpy(out->limbs, alimbs, sizeof(u32) * copy_size);

            add_out = out->limbs + copy_size; 
            alimbs += copy_size;
            asize -= copy_size;
        } else {
            // b extends beyond a
            // aaaa
            //   bbbbb

            out_size = bsize + exp_diff;

            u32 copy_size = bsize + exp_diff - asize;
            memcpy(out->limbs, blimbs, sizeof(u32) * copy_size);

            add_out = out->limbs + copy_size;
            blimbs += copy_size;
            bsize -= copy_size;
        }
        out->exp = a->exp;

        u32 carry = 0;
        for (u32 i = 0; i < bsize; i++) {
            u64 sum = (u64)alimbs[i] + blimbs[i] + carry;

            add_out[i] = (u32)(sum & BIGFLOAT_MASK);
            carry = (u32)(sum >> 32);
        }
        for (u32 i = bsize; i < asize; i++) {
            u64 sum = (u64)alimbs[i] + carry;

            add_out[i] = (u32)(sum & BIGFLOAT_MASK);
            carry = (u32)(sum >> 32);
        }

        out->limbs[out_size] = carry;
        out_size += carry;
        out->exp += carry;
    } else {
        // a and b do not overlap
        // aaa
        //     bb

        memcpy(out->limbs, blimbs, sizeof(u32) * bsize);
        
        if (real_bsize < 0) {
            memset(out->limbs, 0, sizeof(u32) * (exp_diff - asize + real_bsize));
        } else {
            memset(out->limbs + real_bsize, 0, sizeof(u32) * (exp_diff - asize));
        }
        
        memcpy(out->limbs + real_bsize + exp_diff - asize, alimbs, sizeof(u32) * asize);

        out_size = real_bsize + exp_diff;
        out->exp = a->exp;
    }

    mga_scratch_release(scratch);
    
    out->size = a->size < 0 ? -out_size : out_size;
}
void bf_sub(bigfloat* out, const bigfloat* a, const bigfloat* b) {
    if (bf_is_zero(a)) {
        bf_set(out, b);
        out->size *= -1;
        
        return;
    }
    if (bf_is_zero(b)) {
        bf_set(out, a);
        
        return;
    }
    
    if ((a->size ^ b->size) < 0) {
        // a and b hav different signs

        // a - (-b) = a + b
        // -a - b = -a + (-b)

        bigfloat neg_b = {
            .prec = b->prec,
            .size = -b->size,
            .exp = b->exp,
            .limbs = b->limbs
        };

        bf_add(out, a, &neg_b);
        
        return;
    }

    i32 sign = SIGN(a->size);

    if (a->exp < b->exp) {
        const bigfloat* temp = a;
        a = b;
        b = temp;

        sign *= -1;
    }

    u32 asize = ABS(a->size);
    u32 bsize = ABS(b->size);
    i32 real_bsize = bsize;
    u32* alimbs = a->limbs;
    u32* blimbs = b->limbs;
    i32 exp_diff = a->exp - b->exp;
    
    i32 exp = a->exp;

    // Remove parts of a and b that extend beyond prec
    if (asize > out->prec) {
        alimbs += (asize - out->prec);
        asize = out->prec;
    }

    if (bsize + exp_diff > out->prec) {
        blimbs += (bsize + exp_diff) - out->prec;
        
        real_bsize = (i32)out->prec - exp_diff;
        bsize = MAX(0, real_bsize);
    }

    if (exp_diff > (i64)out->prec) {
        // b is canceled

        if (out != a) {
            memcpy(out->limbs, alimbs, sizeof(u32) * asize);
        }
        out->size = asize * sign;
        out->exp = exp;

        return;
    }
    
    mga_temp scratch = mga_scratch_get(NULL, 0);
    if (out == a) {
        u32* tmp_limbs = alimbs;
        alimbs = MGA_PUSH_ARRAY(scratch.arena, u32, asize);
        memcpy(alimbs, tmp_limbs, sizeof(u32) * asize);
    }
    if (out == b) {
        u32* tmp_limbs = blimbs;
        blimbs = MGA_PUSH_ARRAY(scratch.arena, u32, bsize);
        memcpy(blimbs, tmp_limbs, sizeof(u32) * bsize);
    }

    if ((i64)asize > exp_diff) {
        // a and b partially overlap

        u32* sub_out = out->limbs;

        if (exp_diff == 0) {
            bigfloat abs_a = *a;
            bigfloat abs_b = *b;
            abs_a.size = ABS(a->size);
            abs_b.size = ABS(b->size);
            if (bf_cmp(&abs_a, &abs_b) < 0) {
                u32 temp_asize = asize;
                asize = bsize;
                bsize = temp_asize;

                u32* temp_alimbs = alimbs;
                alimbs = blimbs;
                blimbs = temp_alimbs;

                sign *= -1;
            }
            
            if (asize < bsize) {
                // b extends beyond a
                // aaa
                // bbbbb

                out->size = bsize;

                u32 offset = bsize - asize;

                u32 borrow = 0;
                for (u32 i = 0; i < offset; i++) {
                    i64 diff = (i64)0 - blimbs[i] - borrow;

                    sub_out[i] = (u32)diff;
                    borrow = diff < 0;
                }
                for (u32 i = offset; i < bsize; i++) {
                    i64 diff = (i64)alimbs[i - offset] - blimbs[i] - borrow;

                    sub_out[i] = (u32)diff;
                    borrow = diff < 0;
                }
            } else {
                // a extends beyond b
                // aaaaa
                // bbb
                
                out->size = asize;
    
                u32 copy_size = asize - bsize;
                memcpy(out->limbs, alimbs, sizeof(u32) * copy_size);
                
                sub_out += copy_size;
                alimbs += copy_size;
                asize -= copy_size;
                
                u32 borrow = 0;
                for (u32 i = 0; i < bsize; i++) {
                    i64 diff = (i64)alimbs[i] - blimbs[i] - borrow;
    
                    sub_out[i] = (u32)diff;
                    borrow = diff < 0;
                }
            }
        } else { // exp_diff != 0
            u32 borrow = 0;

            if (asize > bsize + exp_diff) {
                // aaaaa
                //   bb 

                out->size = asize;

                u32 copy_size = asize - (bsize + exp_diff);
                memcpy(sub_out, alimbs, sizeof(u32) * copy_size);

                sub_out += copy_size;
                alimbs += copy_size;
                asize = (u32)MAX(0, (i64)asize - copy_size);
            } else {
                // aaaa
                //   bbbb

                out->size = bsize + exp_diff;

                u32 sub_size = bsize + exp_diff - asize;

                for (u32 i = 0; i < sub_size; i++) {
                    i64 diff = (i64)0 - blimbs[i] - borrow;

                    sub_out[i] = (u32)diff;
                    borrow = diff < 0;
                }

                sub_out += sub_size;
                blimbs += sub_size;
                bsize = (u32)MAX(0, (i64)bsize - sub_size);
            }

            for (u32 i = 0; i < bsize; i++) {
                i64 diff = (i64)alimbs[i] - blimbs[i] - borrow;

                sub_out[i] = (u32)diff;
                borrow = diff < 0;
            }
            for (u32 i = bsize; i < asize; i++) {
                i64 diff = (i64)alimbs[i] - borrow;

                sub_out[i] = (u32)diff;
                borrow = diff < 0;
            }
        }
    } else {
        // a and b do not overlap
        // aaaa
        //      bbb
        
        out->size = real_bsize + exp_diff;
        memset(out->limbs, 0, sizeof(u32) * out->size);

        u32 borrow = 0;
        for (u32 i = 0; i < bsize; i++) {
            i64 diff = -(i64)blimbs[i] - borrow;

            out->limbs[i] = (u32)diff;
            borrow = diff < 0;
        }

        u32 size = exp_diff - asize;
        if (real_bsize < 0) {
            size += real_bsize;
        }

        for (u32 i = 0; i < size; i++) {
            out->limbs[i + bsize] = (u32)(-(i64)borrow);
        }

        for (u32 i = 0; i < asize; i++) {
            i64 diff = (i64)alimbs[i] - borrow;

            out->limbs[i + size] = (u32)diff;
            borrow = diff < 0;
        }
    }

    while (out->size > 1 && out->limbs[out->size - 1] == 0) {
        out->size--;
        exp--;
    }

    mga_scratch_release(scratch);

    out->size *= sign;
    out->exp = exp;
    
    _bf_fix_leading_zeros_no_exp(out);
}
void bf_mul(bigfloat* out, const bigfloat* a, const bigfloat* b) {
    if (bf_is_zero(a) || bf_is_zero(b)) {
        out->size = 1;
        out->exp = 0;
        memset(out->limbs, 0, sizeof(u32) * out->prec);
    }

    i32 sign = SIGN(a->size) * SIGN(b->size);

    u32 asize = ABS(a->size);
    u32 bsize = ABS(b->size);
    u32* alimbs = a->limbs;
    u32* blimbs = b->limbs;

    if (asize > out->prec) {
        alimbs += (asize - out->prec);
        asize = out->prec;
    }
    if (bsize > out->prec) {
        blimbs += (bsize - out->prec);
        bsize = out->prec;
    }

    mga_temp scratch = mga_scratch_get(NULL, 0);

    u32 temp_size = asize + bsize;
    u32* temp_out = MGA_PUSH_ZERO_ARRAY(scratch.arena, u32, temp_size);

    u32 carry = 0;
    for (u32 i = 0; i < asize; i++) {
        carry = 0;
        for (u32 j = 0; j < bsize; j++) {
            u64 prod = (u64)alimbs[i] * blimbs[j] + temp_out[i + j] + carry;

            temp_out[i + j] = (u32)(prod & BIGFLOAT_MASK);
            carry = (u32)(prod >> 32);
        }
        temp_out[i + bsize] = carry;
    }

    u32 adjust = (temp_out[asize - 1 + bsize] == 0);
    u32 out_size = temp_size - adjust;
    if (out_size > out->prec) {
        temp_out += out_size - out->prec;
        out_size = out->prec;
    }

    out->size = out_size;
    memcpy(out->limbs, temp_out, sizeof(u32) * out_size);

    mga_scratch_release(scratch);

    out->exp = a->exp + b->exp + !adjust;
    out->size *= sign;

    _bf_fix_leading_zeros_no_exp(out);

    b32 is_zero = true;
    for (u32 i = 0; i < out_size; i++) {
        if (out->limbs[i] != 0) {
            is_zero = false;
            break;
        }
    }

    if (is_zero) {
        bf_set_zero(out);
    }
}
static u32 _u32_leading_zeros(u32 x) {
#ifdef __GNUC__
    return (u32)__builtin_clz(x);
#else
    i32 n = 32;
    u32 y;
    
    y = x >> 16; if (y != 0) { n = n - 16; x = y; }
    y = x >>  8; if (y != 0) { n = n -  8; x = y; }
    y = x >>  4; if (y != 0) { n = n -  4; x = y; }
    y = x >>  2; if (y != 0) { n = n -  2; x = y; }
    y = x >>  1; if (y != 0) return n - 2;
    return n - x;
#endif
}
void bf_div(bigfloat* out, const bigfloat* a, const bigfloat* b) {
    if (out->prec < 2) {
        // I cannot be bothered to make this work 
        fprintf(stderr, "Cannot divide to output with prec less that 2\n");
        return;
    }
    
    if (bf_is_zero(b)) {
        fprintf(stderr, "Cannot divide bigfloat by zero");
        return;
    }
    if (bf_is_zero(a)) {
        if (out != a) {
            bf_set(out, a);
        }
        return;
    }

    i32 sign = SIGN(a->size) * SIGN(b->size);
    u32 asize = ABS(a->size);
    u32 bsize = ABS(b->size);
    u32* alimbs = a->limbs;
    u32* blimbs = b->limbs;

    i64 expected_out_size = (i64)asize - bsize + 1;
    i64 out_size = out->prec;

    i64 zeros = (i64)out_size - expected_out_size;

    if (zeros < 0) {
        // Shorten a
        alimbs += -zeros;
        asize -= -zeros;
        zeros = 0;
    }

    mga_temp scratch = mga_scratch_get(NULL, 0);

    u32 norm_asize = asize + zeros + 1;
    u32* norm_a = MGA_PUSH_ZERO_ARRAY(scratch.arena, u32, norm_asize);
    memcpy(norm_a + zeros, alimbs, sizeof(u32) * asize);
    u32* norm_b = MGA_PUSH_ZERO_ARRAY(scratch.arena, u32, bsize);

    u32 shift = _u32_leading_zeros(blimbs[bsize - 1]);
    norm_a[norm_asize - 1] = norm_a[norm_asize - 2] >> (32 - shift);
    for (i64 i = norm_asize - 2; i > 0; i--) {
        norm_a[i] = (norm_a[i] << shift) | (norm_a[i - 1] >> (32 - shift));
    }
    norm_a[0] <<= shift;
    
    for (i64 i = bsize - 1; i > 0; i--) {
        norm_b[i] = (blimbs[i] << shift) | (blimbs[i - 1] >> (32 - shift));
    }
    norm_b[0] = blimbs[0] << shift;

    //memset(out->limbs, 0, sizeof(u32) * out->prec);
    
    // Main loop for long division
    // calculating digits of the quotient
    for (i64 j = (i64)norm_asize - bsize - 1; j >= 0; j--) {
        // estimated quotient digit
        u64 qhat = (((u64)norm_a[bsize + j] << 32) | (u64)norm_a[bsize + j - 1]) / norm_b[bsize - 1];
        //  remainder of estimated quotient
        u64 rhat = (((u64)norm_a[bsize + j] << 32) | (u64)norm_a[bsize + j - 1]) % norm_b[bsize - 1];
        
        // initial correction of qhat
        // checking if qhat is still correct for the next digit (I think)
        if (bsize >= 2) {
            while (qhat >= BIGFLOAT_BASE || qhat * norm_b[bsize - 2] > ((rhat << 32) | norm_a[norm_asize + j - 3])) {
                qhat--;
                rhat += norm_b[bsize - 1];
                
                if (rhat >= BIGFLOAT_BASE)
                    break;
            }   
        }
        

        if (qhat == 0)
            continue;
        
        // multiply and subtract
        // a -= qhat * b
        i64 carry = 0, diff = 0;
        for (u32 i = 0; i < bsize; i++) {
            u64 product = (u64)norm_b[i] * qhat;
            diff = (i64)norm_a[i + j] - carry - (product & BIGFLOAT_MASK);
            norm_a[i + j] = (u32)diff; // I guess it is okay for it to overflow

            // carry from multiply - subtraction borrow
            carry = (product >> 32) - (diff >> 32);
        }
        diff = norm_a[j + bsize] - carry;
        norm_a[j + bsize] = (u32)diff;

        // Last subtraction needed a borrow
        // subtract one from qhat, add back one b
        if (diff < 0) {
            qhat--;
            
            carry = 0;
            for (u32 i = 0; i < bsize; i++) {
                u64 sum = (u64)norm_b[i] + (u64)norm_a[i + j] + carry;
                norm_a[i + j] = (u32)(sum & BIGFLOAT_MASK);
                carry = sum >> 32;
            }
            norm_a[j + bsize] += carry;
        }

        out->limbs[j] = (u32)qhat;
    }

    mga_scratch_release(scratch);

    out->exp = a->exp - b->exp + 1;
    out->size = sign * out_size;
    _bf_fix_leading_zeros(out);
}

b32 bf_is_zero(const bigfloat* bf) {
    return ABS(bf->size) == 1 && bf->prec > 0 && bf->limbs[0] == 0;
}
b32 bf_equals(const bigfloat* a, const bigfloat* b) {
    if (a->exp != b->exp || a->size != b->size) {
        return false;
    }

    u32 abs_size = ABS(a->size);

    for (u32 i = 0; i < abs_size; i++) {
        if (a->limbs[i] != b->limbs[i])
            return false;
    }

    return true;
}
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

    for (; ai >= 0 && bi >= 0; ai--, bi--) {
        if (a->limbs[ai] != b->limbs[bi]) {
            return (a->limbs[ai] > b->limbs[bi] ? 1 : -1) * sign;
        }
    }

    return 0;
}

static const char* _hex_chars = "0123456789abcdef";
static string8 _bf_to_hex_str(mg_arena* arena, const bigfloat* bf);
string8 bf_to_str(mg_arena* arena, const bigfloat* bf, u32 base) {
    if (bf_is_zero(bf)) {
        return str8_copy(arena, STR8("0"));
    }

    if (base == 16) {
        return _bf_to_hex_str(arena, bf);
    }

    fprintf(stderr, "Unsupported base %u for bf_to_str\n", base);
    return (string8){ 0 };
}
static string8 _bf_to_hex_str(mg_arena* arena, const bigfloat* bf) {
    u32 abs_size = ABS(bf->size);

    mga_temp scratch = mga_scratch_get(&arena, 1);

    u32* limbs = bf->limbs;

    string8 decimal_str = { 0 };
    string8 integer_str = STR8("0");

    i64 num_decimal = (i64)abs_size - bf->exp - 1;

    if (num_decimal > 0) {
        decimal_str.str = MGA_PUSH_ARRAY(scratch.arena, u8, num_decimal * 8);

        u32 shifts = 8;
        u32 cur_limb = *limbs;
        if (cur_limb != 0) {
            while ((cur_limb & 0xf) == 0) {
                cur_limb >>= 4;
                shifts--;
            }
        }

        u32 num_decimal_limbs = MIN(abs_size, num_decimal);
        for (u32 i = 0; i < num_decimal_limbs; i++, limbs += 1, cur_limb = *limbs) {

            for (u32 j = 0; j < shifts; j++) {
                decimal_str.str[decimal_str.size++] = _hex_chars[cur_limb & 0xf];
                cur_limb >>= 4;
            }

            shifts = 8;
        }

        for (u32 i = num_decimal_limbs; i < num_decimal; i++) {
            for (u32 j = 0; j < 8; j++) {
                decimal_str.str[decimal_str.size++] = '0';
            }
        }
    }

    i64 num_integer = (i64)bf->exp + 1;

    if (num_integer > 0) {
        integer_str.str = MGA_PUSH_ARRAY(scratch.arena, u8, num_integer * 8);
        integer_str.size = 0;

        u32 num_integer_limbs = MIN(abs_size, num_integer);

        for (i64 i = 0; i < num_integer - num_integer_limbs; i++) {
            for (u32 j = 0; j < 8; j++) {
                integer_str.str[integer_str.size++] = '0';
            }
        }

        u32 cur_limb = *limbs;
        for (u32 i = 0; i < num_integer_limbs; i++, limbs += 1, cur_limb = *limbs) {
            for (u32 j = 0; j < 8; j++) {
                integer_str.str[integer_str.size++] = _hex_chars[cur_limb & 0xf];
                cur_limb >>= 4;
            }
        }

        while (integer_str.size > 0 && integer_str.str[integer_str.size - 1] == '0') {
            integer_str.size--;
        }
    }

    u64 out_size = integer_str.size + decimal_str.size + (decimal_str.size != 0) + (bf->size < 0);

    string8 out = {
        .size = out_size,
        .str = MGA_PUSH_ZERO_ARRAY(arena, u8, out_size)
    };
    u64 pos = 0;

    if (bf->size < 0) {
        out.str[pos++] = '-';
    }

    for (i64 i = integer_str.size - 1; i >= 0; i--) {
        out.str[pos++] = integer_str.str[i];
    }

    if (decimal_str.size != 0) {
        out.str[pos++] = '.';

        for (i64 i = decimal_str.size - 1; i >= 0; i--) {
            out.str[pos++] = decimal_str.str[i];
        }
    }

    mga_scratch_release(scratch);

    return out;
}
void bf_print(const bigfloat* bf, u32 base) {
    mga_temp scratch = mga_scratch_get(NULL, 0);

    string8 str = bf_to_str(scratch.arena, bf, base);
    printf("%.*s\n", (int)str.size, str.str);

    mga_scratch_release(scratch);
}

// TODO: signs not working?
static void _bf_fix_leading_zeros(bigfloat* bf) {
    u32 abs_size = ABS(bf->size);
    u32 least_sig_digit = 0;
    for (u32 i = 0; i < abs_size; i++) {
        if (bf->limbs[i] != 0) {
            least_sig_digit = i;
            break;
        }
    }

    if (least_sig_digit == 0) {
        return;
    }

    bf->exp += least_sig_digit;
    abs_size -= least_sig_digit;

    for (u32 i = 0; i < abs_size; i++) {
        bf->limbs[i] = bf->limbs[i + least_sig_digit];
    }

    bf->size = abs_size * SIGN(bf->size);
}

static void _bf_fix_leading_zeros_no_exp(bigfloat* bf) {
    u32 abs_size = ABS(bf->size);
    u32 least_sig_digit = 0;
    for (u32 i = 0; i < abs_size; i++) {
        if (bf->limbs[i] != 0) {
            least_sig_digit = i;
            break;
        }
    }

    if (least_sig_digit == 0) {
        return;
    }

    abs_size -= least_sig_digit;

    for (u32 i = 0; i < abs_size; i++) {
        bf->limbs[i] = bf->limbs[i + least_sig_digit];
    }

    bf->size = abs_size * SIGN(bf->size);
}
