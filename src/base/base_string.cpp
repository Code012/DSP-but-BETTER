
/////////////////////////
// C-String Measurement

constexpr U64 cstring8_length(char const* c)
{
	char const* p = c;
	for (;*p != 0; p += 1);
        return (p - c);
}

inline String8 str8_cstring(char const* c_string)
{
	return String8{c_string, cstring8_length(c_string)};
}


/////////////////////////
// String Matching Helpers

constexpr B32 str8_match(String8 const& a, String8 const& b)
{
	B32 result = 0;
	if (a.length == b.length)
    {
        result = MemoryMatch((void const*)a.value, (void const*)b.value, b.length);
    }
    return result;
}

constexpr B32 str8_match_cstr(char const* a_cstr, String8 const& b)
{
	return str8_match(str8_cstring(a_cstr), b);
}

/////////////////////////
// String Types

String8 Str8Builder::string() const
{
	return String8{buffer, length};
}

