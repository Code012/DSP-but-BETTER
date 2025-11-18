/* date = November 18th 2025 9:20 pm */

#define STB_SPRINTF_IMPLEMENTATION
#include "third_party/stb/stb_sprintf.h"


///////////////////////////////
//~ Character Classification & Conversion Functions

internal B32 
CharIsApha(U8 c)
{
	return CharIsAphaUpper(c) || CharIsAlphaLower(c);
}

internal B32 
CharIsAlphaUpper(U8 c)
{
	return c >= 'A' && c <= 'Z';
}

internal B32
CharIsAlphaLower(U8 c)
{
	return c >= 'a' && c <= 'z';
}

internal B32 
CharIsDigit(U8 c)
{
	return c >= '0' && c <= '9';
}

internal B32 
CharIsSpace(U8 c)
{
	return c == ' ' || c == '\t' || c == '\r' || c == '\n' || c == '\f' || c == '\v';
}

internal B32 
UpperFromChar(U8 c)
{
	return (c >= 'a' && c <= 'z') ? ('A' + (c - 'a')) : c;
}
internal B32 
LowerFromChar(U8 c)
{
	return (c >= 'A' && c <= 'Z') ? ('a' + (c - 'A')) : c;
}


////////////////////////////////
//~ C-String Measurement

internal U64 
CString8Length(U8 *cstr)
{
	U8 *p = cstr;
	if (cstr)
	{
		for (;*p != 0; p += 1);
			length = (U64)(p-c);
	}
	return length;
}

////////////////////////////////
//~ String Constructors

internal String8 
Str8(U8 *str, U64 size)
{
	String8 result = {str, size};
	return result;
}

internal String8 
Str8Range(U8 *first, U8* one_past_last)
{
	String8 result = {first, (U64)(one_past_last - first)};
	return result;
}

internal String8 
Str8Zero(void)
{
	String8 result = zero_struct;
	return result;
}

internal String8 
Str8CString(char *c)
{
	String8 result = {(U8 *)c, CString8Length((U8 *)c)};
	return result
}

internal String8 
Str8CStringCapped(void *cstr, void *cap)
{
	char *ptr = (char *)cstr;
	char *opl = (char *)cap;
	for (;ptr < opl && *ptr != 0; ptr += 1);
	U64 size = (U64)(ptr - (char *)cstr);
	String8 result = str8((U8*)cstr, size);
	return result;
}


///////////////////////////////
//~ String Matching

internal B32 
Str8Match(String8 a, String8 b, StringMatchFlags flags)
{
	B32 result = 0;
	B32 no_flags = StringMatchFlags::None;
	if (a.size == b.size && flags == no_flags)
	{
		result = MemoryMatch(a.str, b.str, b.size);
	}
	else if (a.size == b.size || (flags & StringMatchFlags::StartsWith))
	{
		B32 case_insensitive = (flags & StringMatchFlags::CaseInsensitive);
		B32 slash_insensiive = (flags & StringMatchFlags::SlashInsenitive);
		U64 size 			 = Min(a.size, b.size);
		result = 1;
		for (U64 i = 0; i < size; i += 1)
		{
			U8 ai = a.str[i];
			U8 bi = b.str[i];
			if (case_insensitive)
			{
				ai = UpperFromChar(ai);
				bi = UpperFromChar(bi);
			}
			if (ai != bi)
			{
				result = 0;
				break;
			}
		}
	}
	return result;
}

//////////////////////////////
//~ String Slicing

internal String8 
Str8Substr(String8 str, Rng1U64 range)
{
	range.min = ClampTop(range.min, str.size);
	range.max = ClampTop(range,max, str.size);
	str.str += range.min;
	str.size = Dim1U64(range);
	return str;
}
