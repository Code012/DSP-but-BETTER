#ifndef  BASE_MATH_HPP
#define BASE_MATH_HPP



////////////////////////////////
// Range Types

// 1-range
// originally a union of min,max and v[2] but theres some nasty UB involving it so I had to change it
struct Rng1U64
{
    U64 v[2];

    Rng1U64() = default;
    Rng1U64(U64 min, U64 max) : v{min, max}
    {
        if (min > max)
        {
            v[0] = min;
            v[1] = max;
        }
    }

    U64& min()  { return v[0]; }
    U64& max()  { return v[1]; }
};

struct Rng1S64
{
    S64 v[2];

    Rng1S64() = default;
    Rng1S64(S64 min, S64 max) : v{min, max}
    {
        if (min > max)
        {
            v[0] = max;
            v[1] = min;
        }
    }


    S64& min() { return v[0]; }
    S64& max() { return v[1]; }
};

////////////////////////////////
// Range Ops

internal U64 Dim1U64(Rng1U64 r);

internal S64 Dim1S64(Rng1S64 r);
internal S64 DeltaS64(Rng1S64 r);

#endif // BASE_MATH_HPP
