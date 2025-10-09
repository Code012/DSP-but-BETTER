
///////////////////////////////
// Range Types

// 1-range

U64& Rng1U64::operator[](U64 i)                                 { return i == 0 ? min : max; }


//////////////////////////////
// Range Ops

internal constexpr U64 dim_1u64(Rng1U64 r)                         {U64 c = ((r.max > r.min) ? (r.max - r.min) : 0); return c;}
