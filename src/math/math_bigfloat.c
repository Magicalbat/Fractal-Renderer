#include "math_bigfloat.h"

#include <math.h>
#include <stdio.h>
#include <string.h>

bigfloat bf_from_f32(mg_arena* arena, f32 num, u32 prec);
bigfloat bf_from_f64(mg_arena* arena, f64 num, u32 prec);
static bigfloat bf_from_hex_str(mg_arena* arena, string8 str, u32 prec);
bigfloat bf_from_str(mg_arena* arena, string8 str, u32 base, u32 prec) {
    if (base == 16)
        return bf_from_hex_str(arena, str, prec);

    fprintf(stderr, "Unsuporrted base %u", base);
    return (bigfloat) { 0 };
}
static bigfloat bf_from_hex_str(mg_arena* arena, string8 str, u32 prec) {
    // TODO: create util function to "correct" bigfloat 
    return (bigfloat){ 0 };
}

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
