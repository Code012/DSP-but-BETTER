#include "parser.hpp"

Parser::Parser(Lexer& l) : lexer(l) {
    lexer.lex();

    registerPrefix(token::SYMBOL, parseVariable);
    registerPrefix(token::INT, parseNumber);
    registerPrefix(token::MINUS, parsePrefixExpression);
    registerPrefix(token::LPAREN, parseGroupedExpression);
    
    registerInfix(token::PLUS, parseNaryExpression);
    registerInfix(token::MULT, parseNaryExpression);
    registerInfix(token::IMPLICIT_MULT, parseNaryExpression);
    registerInfix(token::MINUS, parseInfixExpression);
    registerInfix(token::DIV, parseInfixExpression);


    nextToken();
    nextToken();

    
};


void Parser::nextToken() {
    curToken = peekToken;
    peekToken = lexer.getNextToken();
}

bool Parser::curTokenIs(const TokenType& t) {
    return curToken.Type == t;
}

bool Parser::peekTokenIs(const TokenType& t) {
    return peekToken.Type == t;
}

bool Parser::match(const TokenType& t) {
    if (peekTokenIs(t)) {
        nextToken();
        return true;
    } else {
        peekError(t);
        return false;
    }
}

std::vector<std::string> Parser::getErrors() {
    return errors;
};

void Parser::peekError(const TokenType& t) {
    std::string msg = "expected next token to be " + t + ", got " + peekToken.Type + " instead";
};

void Parser::noPrefixParseFnError(TokenType& t) {
    std::string msg = "no prefix parse funtion for " + t + " found";
    errors.push_back(msg);
}

std::unique_ptr<ExpressionNode> Parser::parseLoop() {
    std::unique_ptr<ExpressionNode> expr;
    // while(curTokenIs(token::EOL) == false) {
        expr = parseMain();
        // nextToken();
    // }

    return expr;
}

std::unique_ptr<ExpressionNode> Parser::parseMain() {
    // if (curToken.Type == token::FUNC) { // for future
    //     return parseFunc();
    // } else {
    //     return HandleExpression();
    // }

    return HandleExpression();
}

// std::unique_ptr<ExpressionNode> parseFunc() {
//     return;
// }

std::unique_ptr<ExpressionNode> Parser::HandleExpression() {
    auto expr = parseExpression(LOWEST);

    return expr;
    
};

/*
First parse the NUD,
While we haven't reached the end and the bp of the next token is > bp of current token
    Continue parsing left hand side
*/
/**
 * @brief Parses an expression using Pratt parsing (Top-Down Operator Precedence).
 * 
 * Overall algorithm:
 * 1. Parse the NUD (e.g., variable, number, unary minus).
 * 2. While we haven't reached the end of input AND the peek token has higher precedence:
 *    - Parse the right-hand side with the appropriate LED (e.g., +, *, etc.)
 * 
 * @param precedence The binding power (precedence level) of the current context.
 * @return A unique pointer to the root of the parsed expression tree.
 */
std::unique_ptr<ExpressionNode> Parser::parseExpression(int precedence) {
    // If the relevant prefix function (NUD) exists for the token, then
    // then get the function pointer to it and call it and store in leftExpr
    std::unique_ptr<ExpressionNode> leftExpr;

    //* Step 1: Parse the prefix (NUD) — this becomes the initial left-hand side (LHS) of the expression
    if (prefixParseFnList.count(curToken.Type)) { 
        // Find corresponding NUD function for the current token's type
        prefixParseFn prefix = prefixParseFnList.at(curToken.Type);
        // Call the corresponding NUD function to parse current token
        leftExpr = (this->*prefix)();
    } else {
        // Log error if corresponding function isnt found
        noPrefixParseFnError(curToken.Type);
        return nullptr;
    }

    //* Step 2: Check for Implicit Multiplication between 
    //* 2x, 2(expression) and x(expression) pairs
    if ((curTokenIs(token::INT) && peekTokenIs(token::SYMBOL)) ||  // 2x, 12x, 1232x
       ( (curTokenIs(token::INT) || curTokenIs(token::SYMBOL)) && peekTokenIs(token::LPAREN))) {   // 2(), x()
        std::string ch = "*";
        peekToken = insertToken(ch, token::IMPLICIT_MULT);  // Next token is a multiplication token now
    }

    //* Step 3: Repeatedly parse infix expressions (LED) — attach operators and right-hand expressions
    while ((!peekTokenIs(token::EOL)) && (precedence < peekPrecedence())) {
        infixParseFn infix; 
        
        // Find corresponding LED function for the current token's type
        if (infixParseFnList.count(peekToken.Type)) {
            infix = infixParseFnList.at(peekToken.Type);
        } else {
            // If not found, return the AST as is
            return leftExpr;
        }

        nextToken();

        // Parse the left hand side of the AST
        leftExpr = (this->*infix)(std::move(leftExpr)); // rhs is parsed through this call
    }

    // Return root node
    return leftExpr;
}

Token Parser::insertToken(std::string& ch, const TokenType& type) {
    lexer.insertToken(ch, type);
    return Token{type, ch};
}


int Parser::peekPrecedence() {


    if (precedenceList.count(peekToken.Type)) {
        return precedenceList.at(peekToken.Type);
    } else {
        return LOWEST;
    }
}

int Parser::currentPrecedence() {
    if (precedenceList.count(curToken.Type)) {
        return precedenceList.at(curToken.Type);
    } else {
        return LOWEST;
    }
}


std::unique_ptr<ExpressionNode> Parser::parseVariable () {
    // std::cout << curToken.Literal << std::endl;
    return std::make_unique<VariableExpressionNode>(curToken.Literal, curToken, ExprKind::VAR);
}
// ExpressionNode* parsePrefixExpression() {
//     expr = PrefixExpressionNode();
// }

std::unique_ptr<ExpressionNode> Parser::parseNumber() {

    //TODO: perhaps some error handling for conversion
    return std::make_unique<NumberExpressionNode>(curToken, std::stoi(curToken.Literal), ExprKind::NUM);
}

std::unique_ptr<ExpressionNode> Parser::parsePrefixExpression() { 
    // operand first
    std::unique_ptr<PrefixExpressionNode> expr = std::make_unique<PrefixExpressionNode>(*(curToken.Literal.c_str()), curToken, ExprKind::PRE_UNARY_MINUS);
    nextToken(); // consume prefix operand

    // now number
    expr->Right = parseExpression(UNARY);

    return expr;
}

std::unique_ptr<ExpressionNode> Parser::parseInfixExpression(std::unique_ptr<ExpressionNode> left) {
    //first left and operand

    ExprKind kind;
    if (curToken.Type == token::MINUS) { kind = ExprKind::DIFFERENCE; }
    else if (curToken.Type == token::DIV) { kind = ExprKind::QUOTIENT; }
        

    std::unique_ptr<InfixExpressionNode> expr = std::make_unique<InfixExpressionNode>(curToken, *(curToken.Literal.c_str()), kind, std::move(left), nullptr);

    int precedence = currentPrecedence(); // get lbp
    nextToken();
    expr->Right = parseExpression(precedence);

    return expr;
}

std::unique_ptr<ExpressionNode> Parser::parseNaryExpression(std::unique_ptr<ExpressionNode> left) {
    //first left and operand

    ExprKind kind;
    if (curToken.Type == token::PLUS) { kind = ExprKind::PLUS;} 
    else if (curToken.Type == token::MULT || curToken.Type == token::IMPLICIT_MULT ) { kind = ExprKind::MULTIPLY; }
        
    std::vector<std::unique_ptr<ExpressionNode>> operands;
    operands.push_back(std::move(left));

    std::unique_ptr<NaryExpressionNode> expr = std::make_unique<NaryExpressionNode>(curToken, *(curToken.Literal.c_str()), kind, std::move(operands));

    int precedence = currentPrecedence(); // get lbp
    nextToken();
    expr->Operands.push_back(parseExpression(precedence));

    return expr;
}

std::unique_ptr<ExpressionNode> Parser::parseGroupedExpression() {
    nextToken();

    auto expr = parseExpression(LOWEST);

    if (!match(token::RPAREN)) {
        return nullptr;
    }

    return expr;
}



void Parser::registerPrefix(TokenType toktype, prefixParseFn fn) {
    prefixParseFnList.emplace(toktype, fn);
}   

void Parser::registerInfix(TokenType toktype, infixParseFn fn) {
    infixParseFnList.emplace(toktype, fn);
}