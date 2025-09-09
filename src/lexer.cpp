/*
lexer.cpp
Author: Shahbaz
Date 20/01/2025
*/
#ifndef MATHLY_LEXER_CPP_
#define MATHLY_LEXER_CPP_

#include "lexer.hpp"

// Forward Declarations
bool isLetter(char c);
bool isNumber(char c);


Lexer::Lexer(std::string& i) : input(i) {
    readChar();     // Primes Lexer
}

std::vector<Token> Lexer::lex() {
    size_t tok_index = 0;
    while (ch != '$') {
        ++tok_index;
        tokens.push_back(tokenise(tok_index));       
    }

    tokens.push_back(newToken(token::EOL, ch, tok_index));
    return tokens;
}

Token Lexer::getNextToken() {
    auto token = tokens[tokenPosition];
    tokenPosition += 1;

    return token;
}

const Token Lexer::tokenise(size_t tok_index) {
    Token tok;
    
    skipWhitespace();

    // Match current lexeme (ch) to token
    switch (ch) {
        case '+':
            tok = newToken(token::PLUS, ch, tok_index);
            break;
        case '-':
            tok = newToken(token::MINUS, ch, tok_index);
            break;
        case '*':
            tok = newToken(token::MULT, ch, tok_index);
            break;
        case '/':
            tok = newToken(token::DIV, ch, tok_index);
            break;
        case '(':
            tok = newToken(token::LPAREN, ch, tok_index);
            break;
        case ')':
        tok = newToken(token::RPAREN, ch, tok_index);
        break;
        case '$':
            tok = newToken(token::EOL, ch, tok_index);
            break;
        // If none of the single-character lexemes (above) are matched,
        // check if its a symbol or number.
        // If its not even that, then its ILLEGAL
        default:
            if( isLetter(ch)) {
                tok.Literal = readSymbol();
                tok.Type = token::SYMBOL;
                tok.i = tok_index;
                return tok;
            } else if (isNumber(ch)) {
                tok.Literal = readNumber();
                tok.Type = token::INT;
                tok.i = tok_index;
                return tok;
            } else {
                tok = newToken(token::ILLEGAL, ch, tok_index);
            }
    }

    readChar();
    return tok;
}

void Lexer::skipWhitespace() {
    while (ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r') {
        readChar();
    }
}

void Lexer::readChar() {
    if (peekPosition >= input.length()) {
        ch = 0;
    } else {
        ch = input[peekPosition];
    }
    position = peekPosition;
    peekPosition += 1;
}

char Lexer::peekChar() const {
    if (peekPosition >= input.length()) {
        return 0;
    } else {
        return input[peekPosition];
    }
}

std::string Lexer::readSymbol() {
    int16_t startPos = position;
    while (isLetter(ch)) {
        readChar();
    }
    return input.substr(startPos, position-startPos);
}

std::string Lexer::readNumber() {
    int startPos = position;
    while (isNumber(ch)) {
        readChar();
    }
    return input.substr(startPos, position-startPos);
}

const Token Lexer::newToken(TokenType tokenType, char ch, size_t tok_index) {
    char buffer[2] = {ch, '\0'};        // null terminate string
    return Token{tokenType, std::string(buffer), tok_index};
}

// Helper functions
bool isLetter(char ch)  {
    return ('a' <= ch && ch <= 'z') || ('A' <= ch && ch <= 'Z');
}

bool isNumber(char ch) {
    return '0' <= ch && ch <= '9';
}


#endif