#ifndef MATH_COMPLEX_H
#define MATH_COMPLEX_H

#include "base/base_defs.h"

typedef struct { f32 r, i; } complexf;
typedef struct { f64 r, i; } complexd;

complexf complexf_add(complexf a, complexf b);
complexf complexf_sub(complexf a, complexf b);
complexf complexf_mul(complexf a, complexf b);
complexf complexf_div(complexf a, complexf b);
complexf complexf_scale(complexf c, f32 s);

complexd complexd_add(complexd a, complexd b);
complexd complexd_sub(complexd a, complexd b);
complexd complexd_mul(complexd a, complexd b);
complexd complexd_div(complexd a, complexd b);
complexd complexd_scale(complexd c, f64 s);

#endif // MATH_COMPLEX_H