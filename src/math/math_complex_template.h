#include "base/base_defs.h"

#ifndef CX_BASE_TYPE
#define CX_BASE_TYPE f32
#endif

#ifndef CX_NAME_SUFFIX
#define CX_NAME_SUFFIX _##CX_BASE_TYPE
#endif

#define CX_NAME CONCAT(complex, CX_NAME_SUFFIX)

typedef struct { CX_BASE_TYPE r, i; } CX_NAME;

CX_NAME CONCAT(CX_NAME, _add)(CX_NAME a, CX_NAME b);
CX_NAME CONCAT(CX_NAME, _sub)(CX_NAME a, CX_NAME b);
CX_NAME CONCAT(CX_NAME, _mul)(CX_NAME a, CX_NAME b);
CX_NAME CONCAT(CX_NAME, _div)(CX_NAME a, CX_NAME b);
CX_NAME CONCAT(CX_NAME, _scale)(CX_NAME c, CX_BASE_TYPE s);

#ifdef CX_IMPL

CX_NAME CONCAT(CX_NAME, _add)(CX_NAME a, CX_NAME b) {
    return (CX_NAME){ a.r + b.r, a.i + b.i };
}
CX_NAME CONCAT(CX_NAME, _sub)(CX_NAME a, CX_NAME b) {
    return (CX_NAME){ a.r - b.r, a.i - b.i };
}
CX_NAME CONCAT(CX_NAME, _mul)(CX_NAME a, CX_NAME b) {
    return (CX_NAME){
        a.r * b.r - a.i * b.i,
        a.r * b.i + a.i * b.r
    };
}
CX_NAME CONCAT(CX_NAME, _div)(CX_NAME a, CX_NAME b) {
    CX_BASE_TYPE d = b.r * b.r + b.i * b.i;
    return (CX_NAME){
        (a.r * b.r + a.i * b.i) / d,
        (a.i * b.r - a.r * b.i) / d
    };
}
CX_NAME CONCAT(CX_NAME, _scale)(CX_NAME c, CX_BASE_TYPE s) {
    return (CX_NAME){ c.r * s, c.i * s };
}

#endif