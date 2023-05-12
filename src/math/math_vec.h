#ifndef MATH_VEC_H
#define MATH_VEC_H

#define VEC_BASE_TYPE f32
#define VEC_NAME_SUFFIX f
#include "math_vec_template.h"
#undef VEC_BASE_TYPE
#undef VEC_NAME_SUFFIX

#define VEC_BASE_TYPE f64
#define VEC_NAME_SUFFIX d
#include "math_vec_template.h"
#undef VEC_BASE_TYPE
#undef VEC_NAME_SUFFIX

#endif // MATH_VEC_H