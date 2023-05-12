#include "base/base_defs.h"

#ifndef VEC_BASE_TYPE
#define VEC_BASE_TYPE f32
#endif

#ifndef VEC_NAME_SUFFIX
#define VEC_NAME_SUFFIX _##VEC_BASE_TYPE
#endif

#define VEC2_NAME CONCAT(vec2, VEC_NAME_SUFFIX)
#define VEC3_NAME CONCAT(vec3, VEC_NAME_SUFFIX)
#define VEC4_NAME CONCAT(vec4, VEC_NAME_SUFFIX)

typedef struct { VEC_BASE_TYPE x, y;       } VEC2_NAME;
typedef struct { VEC_BASE_TYPE x, y, z;    } VEC3_NAME;
typedef struct { VEC_BASE_TYPE x, y, z, w; } VEC4_NAME;

VEC2_NAME CONCAT(VEC2_NAME, _add)(VEC2_NAME a, VEC2_NAME b);
VEC2_NAME CONCAT(VEC2_NAME, _sub)(VEC2_NAME a, VEC2_NAME b);
VEC2_NAME CONCAT(VEC2_NAME, _mul)(VEC2_NAME v, VEC_BASE_TYPE s);
VEC2_NAME CONCAT(VEC2_NAME, _div)(VEC2_NAME v, VEC_BASE_TYPE s);
VEC_BASE_TYPE CONCAT(VEC2_NAME, _dot)(VEC2_NAME a, VEC2_NAME b);
VEC_BASE_TYPE CONCAT(VEC2_NAME, _sql)(VEC2_NAME v);
VEC_BASE_TYPE CONCAT(VEC2_NAME, _len)(VEC2_NAME v);
VEC2_NAME CONCAT(VEC2_NAME, _prp)(VEC2_NAME v);
VEC2_NAME CONCAT(VEC2_NAME, _nrm)(VEC2_NAME v);

VEC3_NAME CONCAT(VEC3_NAME, _add)(VEC3_NAME a, VEC3_NAME b);
VEC3_NAME CONCAT(VEC3_NAME, _sub)(VEC3_NAME a, VEC3_NAME b);
VEC3_NAME CONCAT(VEC3_NAME, _mul)(VEC3_NAME v, VEC_BASE_TYPE s);
VEC3_NAME CONCAT(VEC3_NAME, _div)(VEC3_NAME v, VEC_BASE_TYPE s);
VEC_BASE_TYPE CONCAT(VEC3_NAME, _dot)(VEC3_NAME a, VEC3_NAME b);
VEC_BASE_TYPE CONCAT(VEC3_NAME, _sql)(VEC3_NAME v);
VEC_BASE_TYPE CONCAT(VEC3_NAME, _len)(VEC3_NAME v);
VEC3_NAME CONCAT(VEC3_NAME, _crs)(VEC3_NAME a, VEC3_NAME b);
VEC3_NAME CONCAT(VEC3_NAME, _nrm)(VEC3_NAME v);

VEC4_NAME CONCAT(VEC4_NAME, _add)(VEC4_NAME a, VEC4_NAME b);
VEC4_NAME CONCAT(VEC4_NAME, _sub)(VEC4_NAME a, VEC4_NAME b);
VEC4_NAME CONCAT(VEC4_NAME, _mul)(VEC4_NAME v, VEC_BASE_TYPE s);
VEC4_NAME CONCAT(VEC4_NAME, _div)(VEC4_NAME v, VEC_BASE_TYPE s);
VEC_BASE_TYPE CONCAT(VEC4_NAME, _dot)(VEC4_NAME a, VEC4_NAME b);
VEC_BASE_TYPE CONCAT(VEC4_NAME, _sql)(VEC4_NAME v);
VEC_BASE_TYPE CONCAT(VEC4_NAME, _len)(VEC4_NAME v);
VEC4_NAME CONCAT(VEC4_NAME, _nrm)(VEC4_NAME v);

#ifdef VEC_IMPL

#include <math.h>

VEC2_NAME CONCAT(VEC2_NAME, _add)(VEC2_NAME a, VEC2_NAME b) {
    return (VEC2_NAME){ a.x + b.x, a.y + b.y };
}
VEC2_NAME CONCAT(VEC2_NAME, _sub)(VEC2_NAME a, VEC2_NAME b) {
    return (VEC2_NAME){ a.x - b.x, a.y - b.y };
}
VEC2_NAME CONCAT(VEC2_NAME, _mul)(VEC2_NAME v, VEC_BASE_TYPE s) {
    return (VEC2_NAME){ v.x * s, v.y * s };
}
VEC2_NAME CONCAT(VEC2_NAME, _div)(VEC2_NAME v, VEC_BASE_TYPE s) {
    return (VEC2_NAME){ v.x / s, v.y / s };
}
VEC_BASE_TYPE CONCAT(VEC2_NAME, _dot)(VEC2_NAME a, VEC2_NAME b) {
    return a.x * b.x + a.y * b.y;
}
VEC_BASE_TYPE CONCAT(VEC2_NAME, _sql)(VEC2_NAME v) {
    return v.x * v.x + v.y * v.y;
}
VEC_BASE_TYPE CONCAT(VEC2_NAME, _len)(VEC2_NAME v) {
    return (VEC_BASE_TYPE)sqrt(v.x * v.x + v.y * v.y);
}
VEC2_NAME CONCAT(VEC2_NAME, _prp)(VEC2_NAME v) {
    return (VEC2_NAME){ -v.y, v.x };
}
VEC2_NAME CONCAT(VEC2_NAME, _nrm)(VEC2_NAME v) {
    VEC_BASE_TYPE len = CONCAT(VEC2_NAME, _len)(v);
    return (VEC2_NAME){ v.x / len, v.y / len };
}

VEC3_NAME CONCAT(VEC3_NAME, _add)(VEC3_NAME a, VEC3_NAME b) {
    return (VEC3_NAME){ a.x + b.x, a.y + b.y, a.z + b.z };
}
VEC3_NAME CONCAT(VEC3_NAME, _sub)(VEC3_NAME a, VEC3_NAME b) {
    return (VEC3_NAME){ a.x - b.x, a.y - b.y, a.z - b.z };
}
VEC3_NAME CONCAT(VEC3_NAME, _mul)(VEC3_NAME v, VEC_BASE_TYPE s) {
    return (VEC3_NAME){ v.x * s, v.y * s, v.z * s };
}
VEC3_NAME CONCAT(VEC3_NAME, _div)(VEC3_NAME v, VEC_BASE_TYPE s) {
    return (VEC3_NAME){ v.x / s, v.y / s, v.z / s };
}
VEC_BASE_TYPE CONCAT(VEC3_NAME, _dot)(VEC3_NAME a, VEC3_NAME b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}
VEC_BASE_TYPE CONCAT(VEC3_NAME, _sql)(VEC3_NAME v) {
    return v.x * v.x + v.y * v.y + v.z * v.z;
}
VEC_BASE_TYPE CONCAT(VEC3_NAME, _len)(VEC3_NAME v) {
    return (VEC_BASE_TYPE)sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
}
VEC3_NAME CONCAT(VEC3_NAME, _crs)(VEC3_NAME a, VEC3_NAME b) {
    return (VEC3_NAME) {
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y - b.x
    };
}
VEC3_NAME CONCAT(VEC3_NAME, _nrm)(VEC3_NAME v) {
    VEC_BASE_TYPE len = CONCAT(VEC3_NAME, _len)(v);
    return (VEC3_NAME){
        v.x / len, v.y / len, v.z / len
    };
}

VEC4_NAME CONCAT(VEC4_NAME, _add)(VEC4_NAME a, VEC4_NAME b) {
    return (VEC4_NAME){ a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w };
}
VEC4_NAME CONCAT(VEC4_NAME, _sub)(VEC4_NAME a, VEC4_NAME b) {
    return (VEC4_NAME){ a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w };
}
VEC4_NAME CONCAT(VEC4_NAME, _mul)(VEC4_NAME v, VEC_BASE_TYPE s) {
    return (VEC4_NAME){ v.x * s, v.y * s, v.z * s, v.w * s };
}
VEC4_NAME CONCAT(VEC4_NAME, _div)(VEC4_NAME v, VEC_BASE_TYPE s) {
    return (VEC4_NAME){ v.x / s, v.y / s, v.z / s, v.w / s };
}
VEC_BASE_TYPE CONCAT(VEC4_NAME, _dot)(VEC4_NAME a, VEC4_NAME b) {
    return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
}
VEC_BASE_TYPE CONCAT(VEC4_NAME, _sql)(VEC4_NAME v) {
    return v.x * v.x + v.y * v.y + v.z * v.z + v.w * v.w;
}
VEC_BASE_TYPE CONCAT(VEC4_NAME, _len)(VEC4_NAME v) {
    return (VEC_BASE_TYPE)sqrt(v.x * v.x + v.y * v.y + v.z * v.z + v.w * v.w);
}
VEC4_NAME CONCAT(VEC4_NAME, _nrm)(VEC4_NAME v) {
    VEC_BASE_TYPE len = CONCAT(VEC4_NAME, _len)(v);
    return (VEC4_NAME){
        v.x / len, v.y / len, v.z / len, v.w / len
    };
}

#endif