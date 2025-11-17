/* date = November 11th 2025 8:53 pm */

#ifndef BASE_STRING_HPP
#define BASE_STRING_HPP

// TODO(me): Get rid of this once finished development
#include "unity.h" // so clangd knows where symbols are for unity build 

// TODO(me): Implement Unicode Conversions (when you need them)

////////////////////////////////
//~ String Types

struct String8
{
    U8  *str;
    U64  size;
};

////////////////////////////////
//~ String List & Array Types

struct String8Node
{
    String8Node *next;
    String8      string;
};

struct String8List
{
    String8Node *first;
    String8Node *last;
    U64          node_count;
    U64          total_size;
};

struct String8Array
{
    String8 *v;
    U64      count;
    U64      total_size;
};

////////////////////////////////
//~ String Joining Types

struct StringJoin
{
    String8 pre;
    String8 sep;   // seperator
    String8 post;
};

////////////////////////////////
//~ C-String Measurement

internal U64 cstring8_length(U8 *c);

////////////////////////////////
//~ String Constructors

#define str8_lit(S) str8((U8*)(S), sizeof(S) - 1)
#define str8_varg(S) (int)((S).size), ((S).str)     // for variadic functions where the format specifier is "%.*s" meaning an int value (width) is provided before the char string.

#define str8_array(S,C)       str8((U8*)(S), sizeof(*(S)*(C)))
#define str8_array_fixed(S,C) str8((U8*)(S), sizeof(S))
#define str8_struct(S) str8((U8*)(S), sizeof(*(S)))   // struct view

internal String8 str8(U8 *str, U64 size);
internal String8 str8_range(U8 *first, U8* one_past_last);
internal String8 str8_zero(void);
internal String8 str8_cstring(char *c);
internal String8 str8_cstring_capped(void *cstr, void *cap);

#endif // BASE_STRING_HPP