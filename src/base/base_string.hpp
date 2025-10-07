#ifndef BASE_STRING_HPP
#define BASE_STRING_HPP

/////////////////////////
// Forward declarations 
struct String8;

/////////////////////////
// C-String Measurement

constexpr U64 cstring8_length(char const* c);

/////////////////////////
// C-String Constructors

inline String8 str8_cstring(char const* c_string);

/////////////////////////
// String Matching Helpers

constexpr B32 str8_match(String8 const& a, String8 const& b);
constexpr B32 str8_match_cstr(char const* a_cstr, String8 const& b);


/////////////////////////
// String Types

struct String8
{
	char const* value{};
	U64 length{};

	constexpr String8() = default;
	
	constexpr String8(String8 const&) = default;
	constexpr String8(String8&&) = default;
	constexpr String8& operator=(String8 const&) = default;
	constexpr String8& operator=(String8&&) = default;
	

	~String8() = default;

	// construction by string literal
	template <U64 N>
	constexpr String8(char const(&str)[N]) : value(str), length(N - 1) {}
	// construction by pointer
	constexpr String8(char const* v, U64 len) : value(v), length(len) {}

	constexpr const char& operator[](U64 i) const 		{ return value[i]; }
	constexpr B32 operator==(String8 const& b) 			{ return str8_match(*this, b); }
	constexpr B32 operator==(char const* b_cstr) 		{ return str8_match(str8_cstring(b_cstr), *this); }		

	constexpr U64 size() const 							{ return length; }

};

struct Str8Builder
{
	char* buffer{};
	U64 length{};
	U64 capacity{};

	String8 string() const;
};


template <U64 SIZE>
internal void
Str8Build(BumpAllocator<SIZE>& arena, Str8Builder& builder, String8& s)
{
	U64 total_needed = builder.length + s.length;

	if (total_needed > builder.capacity) 
	{
		U64 new_capacity = (builder.capacity * 2 > total_needed) ? builder.capacity * 2 : total_needed;

		char* new_buf = arena.ArenaResize<char>(
			builder.buffer,
			builder.capacity,
			new_capacity);

		if (!new_buf) return;

		builder.buffer = new_buf;
		builder.capacity = new_capacity;
	}

	// Copy string directly into builder buffer
	char* write_ptr = builder.buffer + builder.length;
	MemoryCopy(write_ptr, s.value, s.length);

	builder.length += s.length;
}

template <size_t SIZE>
internal void 
Str8BuildCStr(BumpAllocator<SIZE>& arena, Str8Builder& builder, char const* cstr)
{
    Str8Build<SIZE>(arena, builder, String8{cstr, cstring8_length(cstr)});
}

// Formatter builder null terminates strings, so it can interface with standard library functions
// (snprintf null terminates the string, so no point in fighting the behaviour)
template <size_t SIZE, typename... Args>
internal void 
Str8BuildF(BumpAllocator<SIZE>& arena, Str8Builder& builder, char const* format, Args&&... args)
{
    U64 needed = std::snprintf(nullptr, 0, format, std::forward<Args>(args)...) + 1; // account for '\0' because snprintf adds '\0'
    if (needed < 0) return;

    U64 total = builder.length + needed;
    if (total > builder.capacity)
    {
        U64 new_capacity = Max(builder.capacity*2, total);
        char *new_buf = arena.ArenaResize<char>(
            builder.buffer,
            builder.capacity,
            new_capacity);

        if (!new_buf) return;

        builder.buffer= new_buf;
        builder.capacity = new_capacity;
    }

    char* write_ptr = builder.buffer + builder.length;
    U32 written = std::snprintf(write_ptr, builder.capacity - builder.length, format, std::forward<Args>(args)...);
    
    if (written > 0)
    {
	    builder.length += needed - 1;	// Overwrite null-terminator in next go
	    builder.buffer[builder.length] = '\0';
    }
}

#endif // BASE_STRING_HPP