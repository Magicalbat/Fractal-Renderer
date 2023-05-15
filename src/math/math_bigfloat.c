#include "math_bigfloat.h"
#include <math.h>

/*typedef struct {
    u32 mantissa;
    i16 exponent;
    u16 sign;
} float_parts;

float_parts f32_get_parts(f32 num) {
    float_parts parts = {
        .sign = !!signbit(num)
    };

    int exp = 0;
    float mantissa = frexpf(num, &exp);
    parts.exponent = (i16)exp;
    
    mantissa = ldexpf(mantissa, 24);
    parts.mantissa = (u32)truncf(mantissa);

    return parts;
}
float_parts f64_get_parts(f64 num) {
    float_parts parts = {
        .sign = !!signbit(num)
    };

    int exp = 0;
    double mantissa = frexp(num, &exp);
    parts.exponent = (i16)exp;
    
    mantissa = ldexp(mantissa, 53);
    parts.mantissa = (u32)truncf(mantissa);

    return parts;
}*/

bigfloat bf_from_f32(mg_arena* arena, f32 num, u32 prec) {
    bigfloat out = {
        .prec = prec,
        .limbs = MGA_PUSH_ZERO_ARRAY(arena, u32, prec)
    };

    return out;
}
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