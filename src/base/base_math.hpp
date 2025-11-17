#ifndef  BASE_MATH_HPP
#define BASE_MATH_HPP

// TODO(me): Get rid of this once finished development
#include "unity.h" // so clangd knows where symbols are for unity build 

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

internal constexpr U64 dim_1u64(Rng1U64 r);

#endif // BASE_MATH_HPP
