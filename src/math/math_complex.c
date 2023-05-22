#include "math_complex.h"

complexf complexf_add(complexf a, complexf b) {
    return (complexf){ a.r + b.r, a.i + b.i };
}
complexf complexf_sub(complexf a, complexf b) {
    return (complexf){ a.r - b.r, a.i - b.i };
}
complexf complexf_mul(complexf a, complexf b) {
    return (complexf){
        a.r * b.r - a.i * b.i,
        a.r * b.i + a.i * b.r
    };
}
complexf complexf_div(complexf a, complexf b) {
    f32 d = b.r * b.r + b.i * b.i;
    return (complexf){
        (a.r * b.r + a.i * b.i) / d,
        (a.i * b.r - a.r * b.i) / d
    };
}
complexf complexf_scale(complexf c, f32 s) {
    return (complexf){ c.r * s, c.i * s };
}

complexd complexd_add(complexd a, complexd b) {
    return (complexd){ a.r + b.r, a.i + b.i };
}
complexd complexd_sub(complexd a, complexd b) {
    return (complexd){ a.r - b.r, a.i - b.i };
}
complexd complexd_mul(complexd a, complexd b) {
    return (complexd){
        a.r * b.r - a.i * b.i,
        a.r * b.i + a.i * b.r
    };
}
complexd complexd_div(complexd a, complexd b) {
    f64 d = b.r * b.r + b.i * b.i;
    return (complexd){
        (a.r * b.r + a.i * b.i) / d,
        (a.i * b.r - a.r * b.i) / d
    };
}
complexd complexd_scale(complexd c, f64 s) {
    return (complexd){ c.r * s, c.i * s };
}