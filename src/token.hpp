/*
token.hpp

Author: Shahbaz
Date: 19/01/2025
*/
#ifndef MATHLY_TOKEN_HPP_
#define MATHLY_TOKEN_HPP_

#include <iostream>
#include <map>
#include <cstdio>
#include <cstdint>

// Type alias for token types
using TokenType = std::string; // Using string for debugging clarity, can be changed to int later

/**
 * @class Token
 * @brief Represents a lexical token with its type, raw string (lexeme)
 */
class Token {
    public:
        TokenType Type;         // The type of token
        std::string Literal;    // The exact string from the input (lexeme)
        size_t i;               // The position in token list
};


namespace token {

    const TokenType PLUS = "PLUS";              // '+'
    const TokenType MINUS = "MINUS";            // '-'
    const TokenType MULT = "MULT";              // '*'
    const TokenType DIV = "DIV";                // '/'

    const TokenType PRE_MINUS = "PRE_MINUS";    // '-{expression}'

    // This token gets inserted implicitly for
    // 2(x) and 2x, to signify multiplication
    const TokenType IMPLICIT_MULT = "IMPLICIT_MULT";

    const TokenType INT = "INT";                // '42' 

    // Represents variables, e.g. the x in '2x'
    const TokenType SYMBOL = "SYMBOL";

    const TokenType LPAREN = "LPAREN";          // '('
    const TokenType RPAREN = "RPAREN";          // ')'

    // Error token for unrecognised input
    const TokenType ILLEGAL = "ILLEGAL";       

    // End of line/input marker (input is one line)
    const TokenType EOL = "EOL";
    
}

#endif