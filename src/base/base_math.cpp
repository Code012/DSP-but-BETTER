#pragma once
// TODO(me): Get rid of this once finished development
#include "unity.h" // so clangd knows where symbols are for unity build 
///////////////////////////////
// Range Types


//////////////////////////////
// Range Ops

internal constexpr U64 dim_1u64(Rng1U64 r)                         {U64 c = ((r.max > r.min) ? (r.max - r.min) : 0); return c;}
