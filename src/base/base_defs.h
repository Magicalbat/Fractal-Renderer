#ifndef BASE_DEFS_H
#define BASE_DEFS_H

#include <stdint.h>
#include <stdbool.h>

typedef int8_t   i8;
typedef int16_t  i16;
typedef int32_t  i32;
typedef int64_t  i64;
typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef i8  b8;
typedef i32 b32;

typedef float  f32;
typedef double f64;

#define STATIC_ASSERT(c, label) typedef u8 static_assert_##label [(c) ? 1 : -1]

STATIC_ASSERT(sizeof(f32) == 4, f32_size);
STATIC_ASSERT(sizeof(f64) == 8, f64_size);

#if defined(_WIN32)
#   define PLATFORM_WIN32
#elif defined(__linux__)
#   define PLATFORM_LINUX
#endif

#define UNUSED(x) (void)(x)

#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define MAX(a, b) (((a) > (b)) ? (a) : (b))

#define SLL_PUSH_FRONT(f, l, n) ((f) == 0 ? \
    ((f) = (l) = (n)) :                     \
    ((n)->next = (f), (f) = (n)))           \

#define SLL_PUSH_BACK(f, l, n) ((f) == 0 ? \
    ((f) = (l) = (n)) :                    \
    ((l)->next = (n), (l) = (n)),          \
    ((n)->next = 0))                       \

#define SLL_POP_FRONT(f, l) ((f) == (l) ? \
    ((f) = (l) = 0) :                     \
    ((f) = (f)->next))                    \

#define DLL_PUSH_BACK(f, l, n) ((f) == 0 ? \
    ((f) = (l) = (n), (n)->next = (n)->prev = 0) :  \
    ((n)->prev = (l), (l)->next = (n), (l) = (n), (n)->next = 0))

#define DLL_PUSH_FRONT(f, l, n) DLL_PUSH_BACK(l, f, n)

#define DLL_REMOVE(f, l, n) ( \
    (f) == (n) ? \
        ((f) == (l) ? \
            ((f) = (l) = (0)) : \
            ((f) = (f)->next, (f)->prev = 0)) : \
        (l) == (n) ? \
            ((l) = (l)->prev, (l)->next = 0) : \
            ((n)->next->prev = (n)->prev, \
            (n)->prev->next = (n)->next))

#endif // BASE_DEFS_H
