/* date = November 11th 2025 8:14 pm */

#ifndef BASE_ARENA_HPP
#define BASE_ARENA_HPP

// TODO(me): Get rid of this once finished development
#include "unity.h" // so clangd knows where symbols are for unity build 

// TODO(sb): Set up address sanitiser!!
// TODO(sb): Is there a point to templating the pushing functions, experiment?


////////////////////////////////
// Constants

#if !defined(ARENA_COMMIT_GRANULARITY)
# define ARENA_COMMIT_GRANULARITY KB(4)
#endif

#if !defined(ARENA_DECOMMIT_THRESHOLD)
# define ARENA_DECOMMIT_THRESHOLD MB(64)
#endif

#define ARENA_HEADER_SIZE sizeof(Arena)

////////////////////////////////
// Arena Allocator Types

struct Arena 
{
	U64 pos;
	U64 commit_pos;
	U64 align;
	U64 size;
};
StaticAssert(sizeof(Arena) <= ARENA_HEADER_SIZE, arena_header_size_check);

// Arena view for scratch arenas
struct Temp
{
	Arena *arena;
	U64 pos;
}

////////////////////////////////
// Arena Functions

// arena creation/destruction
internal Arena *ArenaAlloc(U64 size=GB(8));
internal void ArenaRelease(Arena *arena);

// arena push/pop/pos core functions
internal void *ArenaPush(Arena *arena, U64 size, U64 align=8, B32 zero=1);	// By default align to 8 bytes and will zero
internal U64   ArenaPos(Arena *arena);
internal void  ArenaPopTo(Arena *arena, U64 pos);

// arena push/pop helpers
internal void ArenaClear(Arena *arena);
internal void ArenaPop(Arena *arena, U64 amt);

// temporary arena scopes
internal Temp TempBegin(Arena *arena);
internal void TempEnd(Temp temp);

// push helper macros
#define PushArrayNoZeroAligned(arena, type, count, align) (T *)ArenaPush((arena), sizeof(type)*(count), (align), (0))
#define PushArrayAligned(arena, type, count, align)  	  (T *)ArenaPush((arena), sizeof(type)*(count), (align), (1))
#define PushArrayNoZero(arena, type, count) 		      PushArrayNoZeroAligned(arena, type, count, Max(8, AlignOf(type)))
#define PushArray(arena, type, count)  				      PushArrayAligned(arena, type, count, Max(8, AlignOf(type)))

#endif // BASE_ARENA_HPP
