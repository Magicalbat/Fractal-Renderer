#include "math_vec.h"

#include <math.h>

vec2f vec2f_add(vec2f a, vec2f b) {
    return (vec2f){ a.x + b.x, a.y + b.y };
}
vec2f vec2f_sub(vec2f a, vec2f b) {
    return (vec2f){ a.x - b.x, a.y - b.y };
}
vec2f vec2f_mul(vec2f v, f32 s) {
    return (vec2f){ v.x * s, v.y * s };
}
vec2f vec2f_div(vec2f v, f32 s) {
    return (vec2f){ v.x / s, v.y / s };
}
f32 vec2f_dot(vec2f a, vec2f b) {
    return a.x * b.x + a.y * b.y;
}
f32 vec2f_sql(vec2f v) {
    return v.x * v.x + v.y * v.y;
}
f32 vec2f_len(vec2f v) {
    return (f32)sqrt(v.x * v.x + v.y * v.y);
}
vec2f vec2f_prp(vec2f v) {
    return (vec2f){ -v.y, v.x };
}
vec2f vec2f_nrm(vec2f v) {
    f32 len = vec2f_len(v);
    return (vec2f){ v.x / len, v.y / len };
}

vec3f vec3f_add(vec3f a, vec3f b) {
    return (vec3f){ a.x + b.x, a.y + b.y, a.z + b.z };
}
vec3f vec3f_sub(vec3f a, vec3f b) {
    return (vec3f){ a.x - b.x, a.y - b.y, a.z - b.z };
}
vec3f vec3f_mul(vec3f v, f32 s) {
    return (vec3f){ v.x * s, v.y * s, v.z * s };
}
vec3f vec3f_div(vec3f v, f32 s) {
    return (vec3f){ v.x / s, v.y / s, v.z / s };
}
f32 vec3f_dot(vec3f a, vec3f b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}
f32 vec3f_sql(vec3f v) {
    return v.x * v.x + v.y * v.y + v.z * v.z;
}
f32 vec3f_len(vec3f v) {
    return (f32)sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
}
vec3f vec3f_crs(vec3f a, vec3f b) {
    return (vec3f) {
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y - b.x
    };
}
vec3f vec3f_nrm(vec3f v) {
    f32 len = vec3f_len(v);
    return (vec3f){
        v.x / len, v.y / len, v.z / len
    };
}

vec4f vec4f_add(vec4f a, vec4f b) {
    return (vec4f){ a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w };
}
vec4f vec4f_sub(vec4f a, vec4f b) {
    return (vec4f){ a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w };
}
vec4f vec4f_mul(vec4f v, f32 s) {
    return (vec4f){ v.x * s, v.y * s, v.z * s, v.w * s };
}
vec4f vec4f_div(vec4f v, f32 s) {
    return (vec4f){ v.x / s, v.y / s, v.z / s, v.w / s };
}
f32 vec4f_dot(vec4f a, vec4f b) {
    return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
}
f32 vec4f_sql(vec4f v) {
    return v.x * v.x + v.y * v.y + v.z * v.z + v.w * v.w;
}
f32 vec4f_len(vec4f v) {
    return (f32)sqrt(v.x * v.x + v.y * v.y + v.z * v.z + v.w * v.w);
}
vec4f vec4f_nrm(vec4f v) {
    f32 len = vec4f_len(v);
    return (vec4f){
        v.x / len, v.y / len, v.z / len, v.w / len
    };
}

vec2d vec2d_add(vec2d a, vec2d b) {
    return (vec2d){ a.x + b.x, a.y + b.y };
}
vec2d vec2d_sub(vec2d a, vec2d b) {
    return (vec2d){ a.x - b.x, a.y - b.y };
}
vec2d vec2d_mul(vec2d v, f64 s) {
    return (vec2d){ v.x * s, v.y * s };
}
vec2d vec2d_div(vec2d v, f64 s) {
    return (vec2d){ v.x / s, v.y / s };
}
f64 vec2d_dot(vec2d a, vec2d b) {
    return a.x * b.x + a.y * b.y;
}
f64 vec2d_sql(vec2d v) {
    return v.x * v.x + v.y * v.y;
}
f64 vec2d_len(vec2d v) {
    return (f64)sqrt(v.x * v.x + v.y * v.y);
}
vec2d vec2d_prp(vec2d v) {
    return (vec2d){ -v.y, v.x };
}
vec2d vec2d_nrm(vec2d v) {
    f64 len = vec2d_len(v);
    return (vec2d){ v.x / len, v.y / len };
}

vec3d vec3d_add(vec3d a, vec3d b) {
    return (vec3d){ a.x + b.x, a.y + b.y, a.z + b.z };
}
vec3d vec3d_sub(vec3d a, vec3d b) {
    return (vec3d){ a.x - b.x, a.y - b.y, a.z - b.z };
}
vec3d vec3d_mul(vec3d v, f64 s) {
    return (vec3d){ v.x * s, v.y * s, v.z * s };
}
vec3d vec3d_div(vec3d v, f64 s) {
    return (vec3d){ v.x / s, v.y / s, v.z / s };
}
f64 vec3d_dot(vec3d a, vec3d b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}
f64 vec3d_sql(vec3d v) {
    return v.x * v.x + v.y * v.y + v.z * v.z;
}
f64 vec3d_len(vec3d v) {
    return (f64)sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
}
vec3d vec3d_crs(vec3d a, vec3d b) {
    return (vec3d) {
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y - b.x
    };
}
vec3d vec3d_nrm(vec3d v) {
    f64 len = vec3d_len(v);
    return (vec3d){
        v.x / len, v.y / len, v.z / len
    };
}

vec4d vec4d_add(vec4d a, vec4d b) {
    return (vec4d){ a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w };
}
vec4d vec4d_sub(vec4d a, vec4d b) {
    return (vec4d){ a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w };
}
vec4d vec4d_mul(vec4d v, f64 s) {
    return (vec4d){ v.x * s, v.y * s, v.z * s, v.w * s };
}
vec4d vec4d_div(vec4d v, f64 s) {
    return (vec4d){ v.x / s, v.y / s, v.z / s, v.w / s };
}
f64 vec4d_dot(vec4d a, vec4d b) {
    return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
}
f64 vec4d_sql(vec4d v) {
    return v.x * v.x + v.y * v.y + v.z * v.z + v.w * v.w;
}
f64 vec4d_len(vec4d v) {
    return (f64)sqrt(v.x * v.x + v.y * v.y + v.z * v.z + v.w * v.w);
}
vec4d vec4d_nrm(vec4d v) {
    f64 len = vec4d_len(v);
    return (vec4d){
        v.x / len, v.y / len, v.z / len, v.w / len
    };
}
