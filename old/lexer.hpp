/*
lexer.cpp
Author: Shahbaz
Date 20/01/2025
*/

// parser will be much easier to use if 

#ifndef LEXER_HPP
#define LEXER_HPP

#include <iostream>
#include <map>
#include <cstdio>
#include <cstdint>
#include <vector>
#include "token.hpp"



class Lexer {
    public:
        std::string input;
        std::vector<Token> tokens;
        size_t tokenPosition {0};
        size_t position {0};
        size_t peekPosition {0};
        char ch;
    
        Lexer(std::string& input);
        std::vector<Token> lex();
        const Token tokenise(size_t tok_index);
        void insertToken(std::string& ch, TokenType type);
        bool findDuplicate(const Token tok, const size_t cur_i);
        Token getNextToken();
        void skipWhitespace();
        void readChar();
        char peekChar() const;
        std::string readVariable();
        std::string readNumber();
        bool isVariableNext();
        bool isNumberNext();
        const Token newToken(TokenType, char ch, size_t tok_index);
};

// Helper functions
bool isLetter(char c);
bool isNumber(char c);


Lexer::Lexer(std::string& i) : input(i) {
    readChar();
}

std::vector<Token> Lexer::lex() {
    size_t tok_index = 0;
    while (ch != '$') {
        ++tok_index;
        tokens.push_back(tokenise(tok_index));       //TODO: just move the tokenise function in here, and instead of returning a token just push to the tokens vector
    }

    tokens.push_back(newToken(token::EOL, ch, tok_index));
    return tokens;
}

Token Lexer::getNextToken() {
    auto token = tokens[tokenPosition];
    tokenPosition += 1;

    return token;
}

void Lexer::insertToken(std::string& ch, TokenType type) {
    tokens.insert(tokens.begin() + tokenPosition - 1, Token{type, ch});
}

// finds the same token after the current index position
bool Lexer::findDuplicate(const Token tok, const size_t current_index) {

    for (size_t i = current_index; i < tokens.size(); i++) {

        if ((tokens[i].Type == tok.Type) && (tokens[i].Literal == tok.Literal)) {
            return true;
        }
    }

    return false;
}

const Token Lexer::tokenise(size_t tok_index) {
    Token tok;
    
    

    skipWhitespace();

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
        // case '$':
        //     tok = newToken(token::EOL, ch);
        //     break;
        default:
            if( isLetter(ch)) {                
                tok.Literal = readVariable();
                tok.Type = token::VAR;
                tok.i = tok_index;

                // if ( isNumberNext() ) {
                //     tok.Type = token::VAR_NUM;
                // }

                return tok;

            } else if (isNumber(ch)) {                
                tok.Literal = readNumber();
                tok.Type = token::INT;
                tok.i = tok_index;

                // if ( isVariableNext() ) {
                //     tok.Type = token::NUM_VAR;
                // }

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

std::string Lexer::readVariable() {
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

bool Lexer::isVariableNext() {
    if (isLetter( input[position + 1] ))
        return true;
    
    return false;
}

bool Lexer::isNumberNext() {
    if (isNumber( input[position + 1] ))
        return true;
    
    return false;
}



// Helper functions

bool isLetter(char ch)  {
    return ('a' <= ch && ch <= 'z') || ('A' <= ch && ch <= 'Z');
}

bool isNumber(char ch) {
    return '0' <= ch && ch <= '9';
}

const Token Lexer::newToken(TokenType tokenType, char ch, size_t tok_index) {
    char buffer[2] = {ch, '\0'};        // null terminate string
    return Token{tokenType, std::string(buffer), tok_index};
}

#endif