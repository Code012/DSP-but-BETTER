/* date = November 11th 2025 8:53 pm */

#ifndef BASE_STRING_HPP
#define BASE_STRING_HPP

#define STB_SPRINTF_DECORATE(name) ts_stbsp_##name
#include "third_party/stb/stb_sprintf.h"

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
//~ String Matching, Splitting & Joining Types
//TODO(sb): String splitting 

enum class StringMatchFlags : U32
{
    None             = (1u << 0),
    CaseInsensitive  = (1u << 1),
    StartsWith       = (1u << 2)
};

internal inline StringMatchFlags operator|(StringMatchFlags lhs, StringMatchFlags rhs)  {return static_cast<StringMatchFlags>( static_cast<U32>(lhs) | static_cast<U32>(rhs) ); }
internal inline StringMatchFlags operator&(StringMatchFlags lhs, StringMatchFlags rhs)  {return static_cast<StringMatchFlags>( static_cast<U32>(lhs) & static_cast<U32>(rhs) ); }
internal inline StringMatchFlags operator^(StringMatchFlags lhs, StringMatchFlags rhs)  {return static_cast<StringMatchFlags>( static_cast<U32>(lhs) ^ static_cast<U32>(rhs) ); }
internal inline StringMatchFlags operator~(StringMatchFlags val)                        {return static_cast<StringMatchFlags>( ~static_cast<U32>(val) ); }
internal inline StringMatchFlags operator|=(StringMatchFlags lhs, StringMatchFlags rhs) {lhs = lhs | rhs; return lhs; }
internal inline StringMatchFlags operator&=(StringMatchFlags lhs, StringMatchFlags rhs) {lhs = lhs & rhs; return rhs }

struct StringJoin
{
    String8 pre;
    String8 sep;   // seperator
    String8 post;
};

///////////////////////////////
//~ Character Classification & Conversion Functions
// TODO(sb): implement as needed
internal B32 CharIsApha(U8 c);
internal B32 CharIsAlphaUpper(U8 c);
internal B32 CharIsAlphaLower(U8 c);
internal B32 CharIsDigit(U8 c);
internal B32 CharIsSpace(U8 c);
internal B32 UpperFromChar(U8 c);
internal B32 LowerFromChar(U8 c);


////////////////////////////////
//~ C-String Measurement

internal U64 CString8Length(U8 *cstr);

////////////////////////////////
//~ String Constructors

#define Str8Lit(S) str8((U8*)(S), sizeof(S) - 1)
#define Str8Varg(S) (int)((S).size), ((S).str)     // for variadic functions where the format specifier is "%.*s" meaning an int value (width) is provided before the char string.

#define Str8Array(S,C) str8((U8*)(S), sizeof(*(S)*(C)))
#define Str8ArrayFixed(S,C) str8((U8*)(S), sizeof(S))
#define Str8Struct(S) str8((U8*)(S), sizeof(*(S)))   // struct view

internal String8 Str8(U8 *str, U64 size);
internal String8 Str8Range(U8 *first, U8* one_past_last);   // memory view
internal String8 Str8Zero(void);
internal String8 Str8CString(char *c);
internal String8 Str8CStringCapped(void *cstr, void *cap);

///////////////////////////////
//~ String Matching

internal B32 Str8Match(String8 a, String8 b, StringMatchFlags flags);

//////////////////////////////
//~ String Slicing

internal String8 Str8Substr(String8 str, Rng1U64 range);

///////////////////////////////
//~ String Formatting & Copying



#endif // BASE_STRING_HPP