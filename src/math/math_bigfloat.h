#ifndef MATH_BIGFLOAT_H
#define MATH_BIGFLOAT_H

#include "base/base.h"

#define BIGFLOAT_BASE ((u64)1 << 32)

typedef struct {
    u32 prec; // number of digits
    i32 size; // limbs in use, sign of size is sign of number
    i32 exp; // exponent in base BIGFLOAT_BASE
    u32* limbs; // digits of the number
} bigfloat;

bigfloat bf_from_f32(mg_arena* arena, f32 num, u32 prec);
bigfloat bf_from_f64(mg_arena* arena, f32 num, u32 prec);
bigfloat bf_from_str(mg_arena* arena, string8 str, u32 base, u32 prec);

b32 bf_add_ip(bigfloat* out, const bigfloat* a, const bigfloat* b);
b32 bf_sub_ip(bigfloat* out, const bigfloat* a, const bigfloat* b);
b32 bf_mul_ip(bigfloat* out, const bigfloat* a, const bigfloat* b);
b32 bf_div_ip(bigfloat* q, const bigfloat* a, const bigfloat* b, bigfloat* r);

bigfloat bf_add(mg_arena* arena, const bigfloat* a, const bigfloat* b);
bigfloat bf_sub(mg_arena* arena, const bigfloat* a, const bigfloat* b);
bigfloat bf_mul(mg_arena* arena, const bigfloat* a, const bigfloat* b);
bigfloat bf_div(mg_arena* arena, const bigfloat* a, const bigfloat* b, bigfloat* r);

b32 bf_is_zero(const bigfloat* bf);
b32 bf_equals(const bigfloat* a, const bigfloat* b);

string8 bf_to_str(mg_arena* arena, const bigfloat* bf, u32 base);
string8 bf_print(const bigfloat* bf, u32 base);

#endif // MATH_BIGFLOAT_H