#include <stdint.h>

#define internal static
#define global static
#define local static

////////////////////////////////
// Types
typedef uint8_t U8;     // 1-byte long unsigned integer
typedef uint16_t U16;   // 2-byte long unsigned integer
typedef uint32_t U32;   // 4-byte long unsigned integer
typedef uint64_t U64;   // 8-byte long unsigned integer

typedef int8_t S8;      // 1-byte long signed integer
typedef int16_t S16;    // 2-byte long signed integer
typedef int32_t S32;    // 4-byte long signed integer
typedef int64_t S64;    // 8-byte long signed integer
typedef S32 B32;        // boolean

////////////////////////////////
// Mins, Maxes
#define Min(A,B) (((A)<(B))?(A):(B))
#define Max(A,B) (((A)>(B))?(A):(B))

