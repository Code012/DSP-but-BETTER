// /*
// lexer.cpp
// Author: Shahbaz
// Date 20/01/2025
// */

// #include "lexer.hpp"

// // Helper functions
// bool isLetter(char c);
// bool isNumber(char c);


// Lexer::Lexer(std::string& i) : input(i) {
//     readChar();
// }

// const Token Lexer::nextToken() {
//     Token tok;
    
//     skipWhitespace();

//     switch (ch) {
//         case '+':
//             tok = newToken(token::PLUS, ch);
//             break;
//         case '-':
//             tok = newToken(token::MINUS, ch);
//             break;
//         case '*':
//             tok = newToken(token::MULT, ch);
//             break;
//         case '/':
//             tok = newToken(token::DIV, ch);
//             break;
//         case '(':
//             tok = newToken(token::LPAREN, ch);
//             break;
//         case ')':
//         tok = newToken(token::RPAREN, ch);
//         break;
//         case '$':
//             tok = newToken(token::EOL, ch);
//             break;
//         default:
//             if( isLetter(ch)) {
//                 tok.Literal = readIdentifier();
//                 tok.Type = token::VAR;
//                 return tok;
//             } else if (isNumber(ch)) {
//                 tok.Literal = readNumber();
//                 tok.Type = token::INT;
//                 return tok;
//             } else {
//                 tok = newToken(token::ILLEGAL, ch);
//             }
//     }

//     readChar();
//     return tok;
// }

// void Lexer::skipWhitespace() {
//     while (ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r') {
//         readChar();
//     }
// }

// void Lexer::readChar() {
//     if (peekPosition >= input.length()) {
//         ch = 0;
//     } else {
//         ch = input[peekPosition];
//     }
//     position = peekPosition;
//     peekPosition += 1;
// }

// char Lexer::peekChar() const {
//     if (peekPosition >= input.length()) {
//         return 0;
//     } else {
//         return input[peekPosition];
//     }
// }

// std::string Lexer::readIdentifier() {
//     int16_t startPos = position;
//     while (isLetter(ch)) {
//         readChar();
//     }
//     return input.substr(startPos, position-startPos);
// }

// std::string Lexer::readNumber() {
//     int startPos = position;
//     while (isNumber(ch)) {
//         readChar();
//     }
//     return input.substr(startPos, position-startPos);
// }


// // Helper functions

// bool isLetter(char ch)  {
//     return ('a' <= ch && ch <= 'z') || ('A' <= ch && ch <= 'Z');
// }

// bool isNumber(char ch) {
//     return '0' <= ch && ch <= '9';
// }

// const Token Lexer::newToken(TokenType tokenType, char ch) {
//     char buffer[2] = {ch, '\0'};        // null terminate string
//     return Token{tokenType, std::string(buffer)};
// }