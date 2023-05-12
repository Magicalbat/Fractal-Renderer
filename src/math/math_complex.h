#ifndef MATH_COMPLEX_H
#define MATH_COMPLEX_H

#define CX_BASE_TYPE f32
#define CX_NAME_SUFFIX f
#include "math_complex_template.h"
#undef CX_BASE_TYPE
#undef CX_NAME_SUFFIX

#define CX_BASE_TYPE f64
#define CX_NAME_SUFFIX d
#include "math_complex_template.h"
#undef CX_BASE_TYPE
#undef CX_NAME_SUFFIX

#endif // MATH_COMPLEX_H