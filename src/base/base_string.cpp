
/////////////////////////
// C-String Measurement

constexpr U64 cstring8_length(char const* c)
{
	char const* p = c;
	for (;*p != 0; p += 1);
        return (p - c);
}

String8 str8_cstring(char const* c_string)
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

// Constructors
constexpr String8::String8(char const* v, U64 len) : value(v), length(len) {} // by pointer

String8 
Str8Builder::string() const
{
	return String8{buffer, length};
}

constexpr char 
String8::operator[](U64 i) const noexcept	
{ 
	return (i < length) ? value[i] : '\0';
}

constexpr String8 
String8::substr(Rng1U64 range) const noexcept
{
	String8 str{};
	range.min = ClampTop(range.min, length);
	range.max = ClampTop(range.max, length);
	str.value = value + range.min;
	str.length = dim_1u64(range);
	return str;
}

// String Comparison Operators

constexpr B32 operator==(String8 const& a,      String8 const& b)	    noexcept  	{ return  str8_match(a, b); }
constexpr B32 operator==(String8 const& a,      char    const* b_cstr)  noexcept  	{ return  str8_match(str8_cstring(b_cstr), a); }		
constexpr B32 operator==(char    const* a_cstr, String8 const& b)       noexcept  	{ return  str8_match(str8_cstring(a_cstr), b); }		
constexpr B32 operator!=(String8 const& a,      String8 const& b)       noexcept  	{ return !str8_match(a, b); }
constexpr B32 operator!=(String8 const& a,      char    const* b_cstr)  noexcept  	{ return !str8_match(str8_cstring(b_cstr), a); }
constexpr B32 operator!=(char    const* a_cstr, String8 const& b)       noexcept  	{ return !str8_match(str8_cstring(a_cstr), b); }

// Stream Output

std::ostream& operator<<(std::ostream& os, String8 const& s) {
    if (s.value && s.length > 0) 
    {
        os.write(s.value, s.length);
    }
    return os;
}
