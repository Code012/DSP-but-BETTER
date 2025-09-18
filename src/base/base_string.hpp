#ifndef BASE_STRING_HPP
#define BASE_STRING_HPP

//////////////////////
// String Types


struct String8
{
    char const *value{};
    U64 length{};
};

struct Str8Builder
{
    char *buffer{};
    U64 length{};
    U64 capacity{};

    String8 string() const;
};

template <size_t N>
constexpr String8 str8_lit_const(char const (&s)[N])
{
    return String8{ s, N - 1}; 

}

template <size_t SIZE>
internal void 
Str8Build(BumpAllocator<SIZE>& arena, Str8Builder& builder, String8 s)
{
    U64 total_needed = builder.length + s.length + 1;

    if (total_needed > builder.capacity) {
        U64 new_capacity = (builder.capacity * 2 > total_needed) ? builder.capacity * 2 : total_needed;

        char* new_buf = arena.ArenaResize<char>(
            builder.buffer,
            builder.capacity,
            new_capacity);

        if (!new_buf) return;  

        builder.buffer= new_buf;
        builder.capacity = new_capacity;
    }
    
    // Copy string directly into builder buffer
    char* write_ptr = builder.buffer + builder.length;
    MemoryCopy(write_ptr, s.value, s.length);

    builder.length += s.length;
    builder.buffer[builder.length] = '\0';
}

template <size_t SIZE>
internal void 
Str8BuildCStr(BumpAllocator<SIZE>& arena, Str8Builder& builder, char const *cstr)
{
    Str8Build<SIZE>(arena, builder, String8{cstr, std::strlen(cstr)});
}

template <size_t SIZE, typename... Args>
internal void 
Str8BuildF(BumpAllocator<SIZE>& arena, Str8Builder& builder, char const *format, Args&&... args)
{
    U64 needed = std::snprintf(nullptr, 0, format, std::forward<Args>(args)...);
    if (needed < 0) return;

    U64 total = builder.length + needed + 1;
    if (total > builder.capacity)
    {
        U64 new_capacity = (builder.capacity * 2 > total) ? builder.capacity*2 : total;

        char *new_buf = arena.ArenaResize<char>(
            builder.buffer,
            builder.capacity,
            new_capacity);

    if (!new_buf) return;

    builder.buffer= new_buf;
    builder.capacity = new_capacity;
    }
    char *write_ptr = builder.buffer + builder.length;
    std::snprintf(write_ptr, builder.capacity - builder.length, format, std::forward<Args>(args)...);
    builder.length += needed;
    builder.buffer[builder.length] = '\0';
}

//////////////////////
// String List & Array Types

/////////////////////
// String Copying 


#endif // BASE_STRING_HPP
