/*
parse.hpp

Author: 
Data: 25/01/2025
*/
// for writing parser: https://llvm.org/docs/tutorial/MyFirstLanguageFrontend/LangImpl02.html
//                      https://matklad.github.io/2020/04/13/simple-but-powerful-pratt-parsing.html
//                      https://journal.stuffwithstuff.com/2011/03/19/pratt-parsers-expression-parsing-made-easy/ 
//  https://www.youtube.com/watch?v=2l1Si4gSb9A helped in intuitvely understanding pratt parsing
//TODO: include copyright
// // g++ -Wall -std=c++20 -g -O0 -mconsole -o BIN/main Mathly/main.cpp Mathly/lexer.cpp

#include <unordered_map>
#include <vector>
#include <cstdint>
#include "ast.hpp"
#include "lexer.hpp"
#include "token.hpp"ch


// Precedence levels for different operators
const int LOWEST = 0;
const int ADDITIVE = 10;
const int MULTIPLICATIVE = 20;
const int UNARY = 30;
const int IMPLICIT_MULT = 21;   // > MULTIPLICATIVE

// Operator precedence table (left binding power)
static std::map<TokenType, int> precedenceList = {
    {token::PLUS, ADDITIVE},
    {token::MINUS, ADDITIVE},
    {token::DIV, MULTIPLICATIVE},
    {token::MULT, MULTIPLICATIVE},
    {token::IMPLICIT_MULT, IMPLICIT_MULT}
};

// Forward declarations
class ExpressionNode;
class Parser;
uint64_t str_to_u64(std::string new_str);

// Function pointer type aliases for LEDs and NUDs
using prefixParseFn = std::unique_ptr<ExpressionNode> (Parser::*)(void); 
using infixParseFn = std::unique_ptr<ExpressionNode> (Parser::*)(std::unique_ptr<ExpressionNode> );


/**
 * @class Parser
 * @brief Responsible for converting a stream of tokens into an Abstract Syntax Tree (AST).
 * 
 * The Parser implements a Pratt-style parsing approach, using prefix and infix parsing functions
 * registered for each token type. It reads tokens sequentially from the Lexer and recursively
 * builds a tree structure that represents the syntactic structure of mathematical expressions.
 * 
 * Supported features include:
 * - Standard arithmetic operations: +, -, *, /
 * - Implicit multiplication (e.g., "2x" or "3(x + 1)")
 * - Parentheses for grouping
 * - Unary minus (e.g., "-5")
 * - Variable names and integer literals
 * 
 * It maintains a current token and a peek token for lookahead and uses precedence rules to
 * control associativity and grouping during parsing.
 * 
 * Errors encountered during parsing (e.g., unexpected tokens or missing handlers) are collected
 * and stored in the `errors` vector.
 * 
 * @see Lexer
 * @see ExpressionNode
 */
class Parser {
    public:
        Lexer lexer;                                   // lexer from which tokens are retrieved
        std::vector<std::string> errors;

        Token curToken;        
        Token peekToken;                               // Looakahead token

        // Maps token types to their corresponding parsing functions (either nud or led)
        std::unordered_map<TokenType, prefixParseFn> prefixParseFnList;
        std::unordered_map<TokenType, infixParseFn> infixParseFnList;

        Parser(Lexer& l);

        // ********************************
        // **Token navigation and utility**
        // ********************************
        void nextToken();                               
        bool curTokenIs(const TokenType&);
        bool peekTokenIs(const TokenType&);
        // If peek token matches, advances and returns true
        // else logs error and returns false
        bool match(const TokenType&);

        
        // ******************
        // **Error handling**
        // ******************
        std::vector<std::string> getErrors();
        // Logs error if token type doesnt match expected token type
        void peekError(const TokenType&);
        // Logs error for a missing prefix parse function (NUD)
        void noPrefixParseFnError(TokenType&);


        // ****************************
        // **Entry points for parsing**
        // ****************************
        // Parses top-level expression
        std::unique_ptr<ExpressionNode> parseLoop();
        // Handles parsing expressions, functions etc
        std::unique_ptr<ExpressionNode> parseMain();
        // Entry point for handling expressions
        std::unique_ptr<ExpressionNode> HandleExpression();


        // ***************************
        // **Core expression parsing**
        // ***************************
        // Main pratt parser loop
        std::unique_ptr<ExpressionNode> parseExpression(int precedence);

        // Prefix parse functions (NUDS)
        std::unique_ptr<ExpressionNode> parseVariable ();
        std::unique_ptr<ExpressionNode> parseNumber();
        std::unique_ptr<ExpressionNode> parsePrefixExpression();
        std::unique_ptr<ExpressionNode> parseGroupedExpression();

        // Infix parse functions (LEDs)
        std::unique_ptr<ExpressionNode> parseInfixExpression(std::unique_ptr<ExpressionNode> left);
        std::unique_ptr<ExpressionNode> parseNaryExpression(std::unique_ptr<ExpressionNode> left);
        
        // Gets precedence/binding power of current token
        int currentPrecedence();
        // Gets precedence/binding power of next token (peekToken)
        int peekPrecedence();
        
        // Helpers to match token types to function pointers in 
        // prefixParseFnList (NUDs) and infixParseFnList (LEDs)
        void registerPrefix(TokenType toktype, prefixParseFn fn);
        void registerInfix(TokenType toktype, infixParseFn fn);

        // Token insertion (for implicit multiplication)
        Token insertToken(std::string& ch, const TokenType&);


};