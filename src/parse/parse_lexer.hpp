/*
* Note: all this code was written in January of 2025, it has just been refactored to work with my string and arena interface
* Date of refactor: 21/09/2025
*/

#ifndef PARSE_LEXER_HPP
#define PARSE_LEXER_HPP

// to be mvoed somehwere else, for the time being itll remain here
local_persist constexpr size_t TOKEN_ARENA_SIZE = 200 * sizeof(TokenNode);
struct Context
{
    BumpAllocator<KB(10)> string_arena;
    BumpAllocator<TOKEN_ARENA_SIZE> token_arena;
};

/////////////////////
// Lexing/Tokenisation Functions

struct Lexer {
    Context&   ctx;
    String8   input;                          // Full input string
    TokenList tokens{};
    U64       token_position {0};             // For parser
    U64       position {0};                   // Current position in input (points to current char)
    U64       peek_position {0};              // Next position input (points to next char)
    char      ch;

    Lexer(String8& input, Context& context); 

    TokenList Lex();
    
    // Returns tokens one at a time
    Token Tokenise(U64 tok_index);
    void InsertToken(String8 const& ch, TokenType type);   // TODO: implement for parser             
    // skips spaces, tabs, newlines and return carriage
    void SkipWhitespace();     
    // Advances to next character and updates position pointers                 
    void ReadChar();         
    // Looks at char at peekPosition                   
    char PeekChar() const;
    // Reads multi-character user-defined symbol (variable)                      
    String8 ReadSymbol();               
    // Reads multi-character user-defined number
    String8 ReadNumber();                   
    Token NewToken(TokenType, char ch, U64 tok_index);
};

#endif // PARSE_LEXER_HPP