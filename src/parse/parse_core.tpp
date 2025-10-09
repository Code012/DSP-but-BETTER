
////////////////////////////////
// Token List

// Token list construction functions
template <size_t SIZE>
TokenNode& TokenList::TokenListPushArena(BumpAllocator<SIZE>& arena, Token& token)
{
    TokenNode* node = arena.PushArrayNoZero<TokenNode>(1);
    TokenListPushNodeSetToken(node, token);
    return *node;
}


//allocates empty node, pushes onto list and populates value