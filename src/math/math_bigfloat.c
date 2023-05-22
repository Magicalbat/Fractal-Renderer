#include "math_bigfloat.h"

#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

static void _bf_fix_leading_zeros(bigfloat* bf); 

bigfloat bf_create(mg_arena* arena, u32 prec) {
    return (bigfloat){
        .prec = prec,
        .size = 1,
        .exp = 0,
        .limbs = MGA_PUSH_ZERO_ARRAY(arena, u32, prec)
    };
}
bigfloat bf_from_f32(mg_arena* arena, f32 num, u32 prec);
bigfloat bf_from_f64(mg_arena* arena, f64 num, u32 prec);

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

void bf_add_ip(bigfloat* out, const bigfloat* a, const bigfloat* b) {
    assert(out != a && out != b);

    if ((a->size ^ b->size) < 0) {
        // a + (-b) = a - b
        // -a + b = -a - (-b)

        bigfloat neg_b = {
            .prec = b->prec,
            .size = -b->size,
            .exp = b->exp,
            .limbs = b->limbs
        };

        bf_sub_ip(out, a, &neg_b);
        
        return;
    }

    if (a->exp < b->exp) {
        const bigfloat* temp = a;
        a = b;
        b = temp;
    }

    u32 asize = ABS(a->size);
    u32 bsize = ABS(b->size);
    u32* a_limbs = a->limbs;
    u32* b_limbs = b->limbs;
    i32 exp_diff = a->exp - b->exp;

    if (asize > (i64)out->prec) {
        a_limbs += asize - out->prec;
        asize = out->prec;
    }
    // b is somewhere to the left of a
    // exp_diff compensates for this
    if (bsize + exp_diff > (i64)out->prec) {
        b_limbs += bsize + exp_diff - out->prec;
        bsize = (u32)MAX(0, (i64)out->prec - exp_diff);
    }

    if ((i64)asize > exp_diff) {
        // a and b overlap

        u32* add_out = NULL;

        if (bsize + exp_diff <= asize) {
            // a extends beyond b
            // aaaaa
            //   bb

            out->size = asize;

            u32 copy_size = asize - exp_diff - bsize;
            memcpy(out->limbs, a_limbs, sizeof(u32) * copy_size);

            add_out = out->limbs + copy_size; 
            a_limbs += copy_size;
            asize -= copy_size;
        } else {
            // b extends beyond a
            // aaaa
            //   bbbbb

            out->size = bsize + exp_diff;

            u32 copy_size = bsize + exp_diff - asize;
            memcpy(out->limbs, b_limbs, sizeof(u32) * copy_size);

            add_out = out->limbs + copy_size;
            b_limbs += copy_size;
            bsize -= copy_size;
        }
        out->exp = a->exp;

        u32 carry = 0;
        for (u32 i = 0; i < bsize; i++) {
            u64 sum = (u64)a_limbs[i] + b_limbs[i];

            add_out[i] = (u32)(sum & BIGFLOAT_MASK);
            carry = (u32)(sum >> 32);
        }
        for (u32 i = bsize; i < asize; i++) {
            u64 sum = (u64)a_limbs[i] + carry;

            add_out[i] = (u32)(sum & BIGFLOAT_MASK);
            carry = (u32)(sum >> 32);
        }

        out->limbs[out->size] = carry;
        out->size += carry;
        out->exp += carry;
    } else {
        // a and b do not overlap
        // aaa
        //     bb

        memcpy(out->limbs, b_limbs, sizeof(u32) * bsize);
        memset(out->limbs + bsize, 0, sizeof(u32) * (exp_diff - asize));
        memcpy(out->limbs + bsize + exp_diff - asize, a_limbs, sizeof(u32) * asize);

        out->size = bsize + exp_diff;
        out->exp = a->exp;
    }

    out->size *= SIGN(a->size);
}
void bf_sub_ip(bigfloat* out, const bigfloat* a, const bigfloat* b) {
    assert(out != a && out != b);

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

        bf_add_ip(out, a, &neg_b);
        
        return;
    }

    i32 sign = SIGN(a->size);
}
void bf_mul_ip(bigfloat* out, const bigfloat* a, const bigfloat* b);
void bf_div_ip(bigfloat* q, const bigfloat* a, const bigfloat* b, bigfloat* r);

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

static const char* _hex_chars = "0123456789abcdef";
static string8 _bf_to_hex_str(mg_arena* arena, const bigfloat* bf);
string8 bf_to_str(mg_arena* arena, const bigfloat* bf, u32 base) {
    // TODO: make bf_is_zero
    //if (bf_is_zero(bf)) {
    //    return str8_copy(arena, STR8("0"));
    //}

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
        while ((cur_limb & 0xf) == 0) {
            cur_limb >>= 4;
            shifts--;
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

static void _bf_fix_leading_zeros(bigfloat* bf) {
    u32 least_sig_digit = 0;
    for (u32 i = 0; i < bf->prec; i++) {
        if (bf->limbs[i] != 0) {
            least_sig_digit = i;
            break;
        }
    }

    if (least_sig_digit == 0) {
        return;
    }

    bf->exp += least_sig_digit;
    u32 abs_size = ABS(bf->size) - least_sig_digit;

    for (u32 i = 0; i < abs_size; i++) {
        bf->limbs[i] = bf->limbs[i + least_sig_digit];
    }

    bf->size = abs_size * SIGN(bf->size);
}