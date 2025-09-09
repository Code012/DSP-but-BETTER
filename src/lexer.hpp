/*
lexer.cpp
Author: Shahbaz
Date 20/01/2025
*/

#ifndef MATHLY_LEXER_HPP_
#define MATHLY_LEXER_HPP_

#include <iostream>
#include <map>
#include <cstdio>
#include <cstdint>
#include <vector>
#include "token.hpp"


/**
 * @class Lexer
 * @brief Tokenizes input source code into a stream of tokens.
 *
 * The Lexer reads characters from a source string and converts them
 * into a sequence of tokens defined in `token.hpp`. It handles identifiers,
 * integers, operators, and delimiters, skipping irrelevant characters like
 * whitespace.
 *
 * Designed for single-pass left-to-right lexing with single character lookahead capability.
 *
 * Typical usage:
 * @code
 *   std::string source = "a + b * 3";
 *   Lexer lexer(source);
 *   Token tok;
 *   do {
 *       tok = lexer.nextToken();
 *       std::cout << tok.Type << " " << tok.Literal << std::endl;
 *   } while (tok.Type != token::EOL);
 * @endcode
 */
class Lexer {
    public:
        std::string input;                          // Full input string
        std::vector<Token> tokens;                  // Tokenised input list
        size_t tokenPosition {0};                   // For parser
        size_t position {0};                        // Current position in input (points to current char)
        size_t peekPosition {0};                    // Next position input (points to next char)
        char ch;                                    // Current character being processed
    
        Lexer(std::string& input); 

        std::vector<Token> lex();
        // Returns tokens one at a time
        const Token tokenise(size_t tok_index);
        Token getNextToken();
        void insertToken(std::string&ch, TokenType type);                 
        // skips spaces, tabs, newlines and return carriage
        void skipWhitespace();     
        // Advances to next character and updates position pointers                 
        void readChar();         
        // Looks at char at peekPosition                   
        char peekChar() const;
        // Reads multi-character user-defined symbol (variable)                      
        std::string readSymbol();               
        // Reads multi-character user-defined number
        std::string readNumber();                   
        const Token newToken(TokenType, char ch, size_t tok_index);
};


#endif