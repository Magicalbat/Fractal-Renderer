#ifndef MATH_VEC_H
#define MATH_VEC_H

#include "base/base_defs.h"

typedef struct { f32 x, y;       } vec2f;
typedef struct { f32 x, y, z;    } vec3f;
typedef struct { f32 x, y, z, w; } vec4f;

vec2f vec2f_add(vec2f a, vec2f b);
vec2f vec2f_sub(vec2f a, vec2f b);
vec2f vec2f_mul(vec2f v, f32 s);
vec2f vec2f_div(vec2f v, f32 s);
f32 vec2f_dot(vec2f a, vec2f b);
f32 vec2f_sql(vec2f v);
f32 vec2f_len(vec2f v);
vec2f vec2f_prp(vec2f v);
vec2f vec2f_nrm(vec2f v);

vec3f vec3f_add(vec3f a, vec3f b);
vec3f vec3f_sub(vec3f a, vec3f b);
vec3f vec3f_mul(vec3f v, f32 s);
vec3f vec3f_div(vec3f v, f32 s);
f32 vec3f_dot(vec3f a, vec3f b);
f32 vec3f_sql(vec3f v);
f32 vec3f_len(vec3f v);
vec3f vec3f_crs(vec3f a, vec3f b);
vec3f vec3f_nrm(vec3f v);

vec4f vec4f_add(vec4f a, vec4f b);
vec4f vec4f_sub(vec4f a, vec4f b);
vec4f vec4f_mul(vec4f v, f32 s);
vec4f vec4f_div(vec4f v, f32 s);
f32 vec4f_dot(vec4f a, vec4f b);
f32 vec4f_sql(vec4f v);
f32 vec4f_len(vec4f v);
vec4f vec4f_nrm(vec4f v);

typedef struct { f64 x, y;       } vec2d;
typedef struct { f64 x, y, z;    } vec3d;
typedef struct { f64 x, y, z, w; } vec4d;

vec2d vec2d_add(vec2d a, vec2d b);
vec2d vec2d_sub(vec2d a, vec2d b);
vec2d vec2d_mul(vec2d v, f64 s);
vec2d vec2d_div(vec2d v, f64 s);
f64 vec2d_dot(vec2d a, vec2d b);
f64 vec2d_sql(vec2d v);
f64 vec2d_len(vec2d v);
vec2d vec2d_prp(vec2d v);
vec2d vec2d_nrm(vec2d v);

vec3d vec3d_add(vec3d a, vec3d b);
vec3d vec3d_sub(vec3d a, vec3d b);
vec3d vec3d_mul(vec3d v, f64 s);
vec3d vec3d_div(vec3d v, f64 s);
f64 vec3d_dot(vec3d a, vec3d b);
f64 vec3d_sql(vec3d v);
f64 vec3d_len(vec3d v);
vec3d vec3d_crs(vec3d a, vec3d b);
vec3d vec3d_nrm(vec3d v);

vec4d vec4d_add(vec4d a, vec4d b);
vec4d vec4d_sub(vec4d a, vec4d b);
vec4d vec4d_mul(vec4d v, f64 s);
vec4d vec4d_div(vec4d v, f64 s);
f64 vec4d_dot(vec4d a, vec4d b);
f64 vec4d_sql(vec4d v);
f64 vec4d_len(vec4d v);

#endif // MATH_VEC_H
