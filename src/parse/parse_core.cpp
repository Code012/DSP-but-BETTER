
/////////////////////////
// Token List

// Token list construction functions

TokenNode& 
TokenList::TokenListPushNode(TokenNode*& node)
{
    SLLQueuePush(first, last, node);
    node_count += 1;
    return *node;
}

TokenNode&
TokenList::TokenListPushNodeSetToken(TokenNode*& node, Token& token)
{
    SLLQueuePush(first, last, node);
    node_count += 1;
    node->token = token;
    return *node;
}


// Helper functions

bool IsLetter(char ch)  {
    return ('a' <= ch && ch <= 'z') || ('A' <= ch && ch <= 'Z');
}

bool IsNumber(char ch) {
    return '0' <= ch && ch <= '9';
}

char const* ToString(TokenType type)
{
    switch (type)
    {
        case TokenType::PLUS:           return "PLUS";
        case TokenType::MINUS:          return "MINUS";
        case TokenType::MULT:           return "MULT";
        case TokenType::DIV:            return "DIV";
        case TokenType::UNARY_MINUS:    return "UNARY_MINUS";
        case TokenType::IMPLICIT_MULT:  return "IMPLICIT_MULT"; 
        case TokenType::NUM:            return "NUM";
        case TokenType::SYMBOL:         return "SYMBOL";
        case TokenType::LPAREN:         return "LPAREN"; 
        case TokenType::RPAREN:         return "RPAREN"; 
        case TokenType::ILLEGAL:        return "ILLEGAL"; 
        case TokenType::EOL:            return "EOL"; 

        case TokenType::NOT_SET:        return "NOT_SET MATE";
        default:                        return "UNKNOWN";
    }
}
