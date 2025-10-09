#ifndef  BASE_MATH_HPP
#define BASE_MATH_HPP

///////////////////////////////
// Range Types

// 1-range

struct Rng1U64
{
    U64 min;
    U64 max;

    U64& operator[](U64 i);
};


//////////////////////////////
// Range Ops

internal constexpr Rng1U64 rng_1u64(U64 min, U64 max);

#endif // BASE_MATH_HPP
