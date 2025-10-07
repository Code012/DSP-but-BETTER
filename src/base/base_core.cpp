
/////////////////
// Memory Operations
#include "base_core.hpp"
internal void 
MemoryCopy(void *dst, const void *src, std::size_t size)                                  { std::memmove(dst, src, size); }
internal void 
MemoryZero(void *dst, std::size_t size)                                             { std::memset(dst, 0, size); }
internal inline B32
MemoryCompare(const void* lhs, const void* rhs, std::size_t count)                              { return std::memcmp(lhs, rhs, count); }

template <typename T>
inline void 
MemoryZeroStruct(T& obj)                                                            { MemoryZero(&obj, sizeof(T)); }
template <typename T, std::size_t N>
inline void 
MemoryZeroArray(T (&arr)[N])                                                        { MemoryZero(arr, sizeof(arr)); }

internal inline B32 
MemoryMatch(const void* lhs, const void* rhs, std::size_t count)                                { return (MemoryCompare(lhs, rhs, count) == 0); }

internal U64 
DefaultAlign(U64 align)                                                             { return Max<U64>(8, align); }
