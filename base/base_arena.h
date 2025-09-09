#ifndef BASE_ARENA_H
#define BASE_ARENA_H


typedef struct Arena Arena;
struct Arena
{
    unsigned char *memory;
    U64 size;
    U64 prev_offset; // TODO(shahbaz): For resizing function, see gingerbill
    U64 cur_offset;
    int alloc_counter;
};

// Arena creation/destruction
internal Arena *ArenaAlloc(U64 size);
internal void ArenaRelease(Arena *arena);

// Arena push/pos core functions
internal void *ArenaPush(Arena *arena, U64 size, U64 align, B32 zero);
internal U64 ArenaPos(Arena *arena);

// Push helper macros
// a-arena, T-type, c-count
#define PushArrayNoZeroAligned(a, T, c, align) (T *)ArenaPush((a), sizeof(T)*(c), (align), (0))
#define PushArrayAligned(a, T, c, align) (T *)ArenaPush((a), sizeof(T)*(c), (align), (1))
#define PushArrayNoZero(a, T, c) PushArrayNoZeroAligned(a, T, c, Max(8, alignof(T)))
#define PushArray(a, T, c) PushArrayAligned(a, T, c, Max(8, alignof(T)))

#endif // BASE_ARENA_H
