#ifndef BASE_STR_H
#define BASE_STR_H

#include "base_defs.h"
#include "mg_arena/mg_arena.h"

// This is heavily based on the string 
// header in Mr 4th programming series: 
// https://github.com/Mr-4th-Programming/mr4th/blob/main/src/base/base_string.h

typedef struct string8 {
    u8* str;
    u64 size;
} string8;

typedef struct string16 {
    u16* str;
    u64 size;
} string16;

typedef struct string32 {
    u32* str;
    u64 size;
} string32;

typedef struct string8_node {
    struct string8_node* next;
    string8 str;
} string8_node;

typedef struct string8_list {
    string8_node* first;
    string8_node* last;
    u64 node_count;
    u64 total_size;
} string8_list; 

typedef struct string8_join {
    string8 pre;
    string8 inbetween;
    string8 post;
} string8_join;

typedef struct string_decode {
    u32 code_point;
    u32 size;
} string_decode;

#define STR8(s) ((string8){ (u8*)(s), sizeof(s)-1 })

string8 str8_create(u8* str, u64 size);
string8 str8_from_range(u8* start, u8* end);
string8 str8_from_cstr(u8* cstr);

string8 str8_copy(mg_arena* arena, string8 str);
u8*     str8_to_cstr(mg_arena* arena, string8 str);

b8 str8_equals(string8 a, string8 b);
b8 str8_contains(string8 a, string8 b);

u64 str8_find_first(string8 a, string8 b);
u64 str8_find_last(string8 a, string8 b);

string8 str8_prefix(string8 str, u64 size);
string8 str8_postfix(string8 str, u64 size);
string8 str8_substr(string8 str, u64 start, u64 end);
string8 str8_substr_size(string8 str, u64 start, u64 size);

string8 str8_cut_end_until(string8 str, u8 c);

void str8_list_push_existing(string8_list* list, string8 str, string8_node* node);
void str8_list_push(mg_arena* arena, string8_list* list, string8 str);

string8_list str8_split(mg_arena* arena, string8 orig, string8 split);
string8_list str8_split_char(mg_arena* arena, string8 orig, u8 split_char);

string8 str8_concat(mg_arena* arena, string8_list list);
string8 str8_join(mg_arena* arena, string8_list list, string8_join join);

string8 str8_pushfv(mg_arena* arena, const char* fmt, va_list args);
string8 str8_pushf(mg_arena* arena, const char* fmt, ...);

string_decode str_decode_utf8(u8* str, u32 cap);
u32           str_encode_utf8(u8* dst, u32 code_point);

string_decode str_decode_utf16(u16* str, u32 cap);
u32           str_encode_utf16(u16* dst, u32 code_point);

string32 str32_from_str8(mg_arena* arena, string8 str);
string8  str8_from_str32(mg_arena* arena, string32 str);
string16 str16_from_str8(mg_arena* arena, string8 str);
string8  str8_from_str16(mg_arena* arena, string16 str);

#endif // BASE_STR_H
