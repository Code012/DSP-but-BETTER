/*
    9/9/2025
*/

// Arena creation/destruction
internal Arena *ArenaAlloc(U64 size)
{
    Arena arena = {0};
    arena.memory = (unsigned char*)malloc(size);
    return &arena;
}

internal void ArenaRelease(Arena *arena)
{
    free(arena->memory);

    arena->memory = NULL;
    arena->size = 0;
    arena->prev_offset = 0;
    arena->cur_offset = 0;
    arena->alloc_counter = 0;
}

// Arena push/pos core functions
internal void *ArenaPush(Arena *arena, U64 size, U64 align, B32 zero, U64 count)
{
    if (!arena || !size) return NULL;

    arena->alloc_counter += count;

    // Check if allocation within arena size
    if (size > arena->size) return NULL;

    // Calculate and align new pointer pos
    uintptr_t current_pointer_pos = (uintptr_t)arena->memory + (uintptr_t)arena->cur_offset;
    uintptr_t aligned_pointer_pos = (current_pointer_pos + align - 1) & ~(align - 1);

    if (aligned_pointer_pos < current_pointer_pos) return NULL; // Overflow check

    uintptr_t relative_offset = (uintptr_t)arena->memory - aligned_pointer_pos; // offset relative to start

    // Check for space in backing memory
    if (relative_offset+size <= arena->size)
    {
        void *ptr = &arena->memory[relative_offset];
        arena->prev_offset = relative_offset;
        arena->cur_offset = relative_offset+size;

        if (zero) MemoryZero(ptr, size);

        return ptr;
    }

    return NULL;
}

// Get # of bytes allocated
internal U64 ArenaGetPos(Arena *arena)
{
    return arena->cur_offset;
}

// Popping functions 
internal void ArenaSetPosBack(Arena *arena, U64 pos)
{
    arena->cur_offset = pos;
}
internal void ArenaClear(Arena *arena)
{
    arena->cur_offset = 0;
}

