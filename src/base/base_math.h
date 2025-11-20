#ifndef  BASE_MATH_HPP
#define BASE_MATH_HPP



////////////////////////////////
// Range Types

// 1-range

union Rng1U64
{
    struct
    {
        U64 min;
        U64 max;
    };
    U64 v[2];
};

////////////////////////////////
// Range Ops

internal U64 Dim1U64(Rng1U64 r);

#endif // BASE_MATH_HPP
