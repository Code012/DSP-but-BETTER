#ifndef PARSE_CORE_HPP
#define PARSE_CORE_HPP

////////////////////////////
// Foreign Includes

#include <vector>

/////////////////////////////
// Token Types

// get 256 Tokens, if you have time or want to. Explore bit enums for EDUCATIONAL purposes
enum class TokenType : U8 
{
    NOT_SET,                // for zii
    PLUS,
    MINUS,
    MULT,
    DIV,
    UNARY_MINUS,            // -{expression}
    IMPLICIT_MULT,          // 2x, 2(x) -> 2*x; x(2+2) -> x * (2+2) 
    NUM,
    SYMBOL,
    LPAREN,
    RPAREN,
    ILLEGAL,
    EOL
};

// Represents a lexical token with its type, raw string (lexeme)
struct Token 
{
    TokenType type{};         
    String8   literal{};    // (lexeme)
    U64       i{};          // The position in token list
};


////////////////////////////
// Token List

struct TokenNode
{
    TokenNode* next{};
    Token      token{};
};

struct TokenList
{   
    TokenNode* first{};
    TokenNode* last{};
    U64        node_count{};

    // Token list construction functions
    TokenNode& TokenListPushNodeSetToken(TokenNode*& node, Token& token);
    TokenNode& TokenListPushNode(TokenNode*& node);
    template <typename size_t SIZE>
    TokenNode& TokenListPushArena(BumpAllocator<SIZE>& arena, Token& token);
};




//////////////////////////////////
// Helpers

internal bool IsLetter(char c);
internal bool IsNumber(char c);

internal char const* ToString(TokenType type);


#include "parse_core.tpp"

#endif // PARSE_CORE_HPP
