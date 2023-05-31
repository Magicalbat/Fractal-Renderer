#ifndef MATH_BIGFLOAT_H
#define MATH_BIGFLOAT_H

#include "base/base.h"

#define BIGFLOAT_BASE ((u64)1 << 32)
#define BIGFLOAT_MASK (BIGFLOAT_BASE - 1)

typedef struct {
    u32 prec; // number of digits
    i32 size; // limbs in use, sign of size is sign of number
    i32 exp; // exponent in base BIGFLOAT_BASE
    u32* limbs; // digits of the number
} bigfloat;

bigfloat bf_create(mg_arena* arena, u32 prec);
bigfloat bf_copy(mg_arena* arena, bigfloat* bf);
bigfloat bf_from_f64(mg_arena* arena, f64 num, u32 prec);
bigfloat bf_from_i64(mg_arena* arena, i64 num, u32 prec);
bigfloat bf_from_str(mg_arena* arena, string8 str, u32 base, u32 prec);

void bf_set(bigfloat* a, const bigfloat* b);
void bf_set_f64(bigfloat* bf, f64 num);
void bf_set_i64(bigfloat* bf, i64 num);
void bf_set_zero(bigfloat* bf);

void bf_add_ip(bigfloat* out, const bigfloat* a, const bigfloat* b);
void bf_sub_ip(bigfloat* out, const bigfloat* a, const bigfloat* b);
void bf_mul_ip(bigfloat* out, const bigfloat* a, const bigfloat* b);
void bf_div_ip(bigfloat* out, const bigfloat* a, const bigfloat* b);

b32 bf_is_zero(const bigfloat* bf);
b32 bf_equals(const bigfloat* a, const bigfloat* b);
// - -> a < b, 0 -> a == b, + -> a > b
i32 bf_cmp(const bigfloat* a, const bigfloat* b);

string8 bf_to_str(mg_arena* arena, const bigfloat* bf, u32 base);
void bf_print(const bigfloat* bf, u32 base);

#endif // MATH_BIGFLOAT_H
