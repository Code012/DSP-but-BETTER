#ifndef BASE_CORE_H
#define BASE_CORE_H

///////////////////
// Foreign Includes

#include <cstdint>
#include <cstring>
#include <cstddef>
#include <iostream>
#include <limits>

//////////////////
// Codebase Keywords

#define internal static
#define global static
#define local_persist static

//////////////////
// Base Types

using U8 = uint8_t;
using U16 = uint16_t;
using U32 = uint32_t;
using U64 = uint64_t;
using S8 = int8_t;
using S16 = int16_t;
using S32 = int32_t;
using S64 = int64_t;
using B32 = S32; // bool32


//////////////////
// Units

constexpr U64 KB(U64 n) { return n << 10; }
constexpr U64 MB(U64 n) { return n << 20; }
constexpr U64 GB(U64 n) { return n << 30; }
constexpr U64 TB(U64 n) { return n << 40; }
constexpr U64 Thousand(U64 n) { return n*1000; }
constexpr U64 Million(U64 n) { return n*1000000; }
constexpr U64 Billion(U64 n) { return n*1000000000; }

/////////////////
// Clamps, Mins, Maxes

template <typename T>
T Min(T a, T b) { return (a<b)?a:b; }
template <typename T>
T Max(T a, T b) { return (a>b)?a:b;}
template <typename T>
T ClampTop(T a, T x) { return Min<T>(a, x); }
template <typename T>
T ClampBot(T x, T a) { return Max<T>(x, a); }

/////////////////
// Memory Operations

internal inline void 
MemoryCopy(void *dst, const void *src, std::size_t size);
internal inline void 
MemoryZero(void *dst, std::size_t size);

template <typename T>
inline void MemoryZeroStruct(T& obj);
template <typename T, std::size_t N>
inline void MemoryZeroArray(T (&arr)[N]);

U64 DefaultAlign(U64 align);
#endif // BASE_CORE_H
