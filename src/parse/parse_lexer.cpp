
Lexer::Lexer(String8& i, Context& c) : input(i), ctx(c) {
    ReadChar();     // Primes Lexer
}

TokenList Lexer::Lex()
{
    U64 tok_index = 0;
    while (ch != '$') {
        ++tok_index;
        tokens.TokenListPushArena(ctx.token_arena, Tokenise(tok_index));       
    }

    tokens.TokenListPushArena(ctx.token_arena, NewToken(TokenType::EOL, ch, tok_index));
    return tokens;
}

Token Lexer::Tokenise(U64 tok_index) {
    Token tok;
    
    SkipWhitespace();

    // Match current lexeme (ch) to token
    switch (ch) {
        case '+':
            tok = NewToken(TokenType::PLUS, ch, tok_index);
            break;
        case '-':
            tok = NewToken(TokenType::MINUS, ch, tok_index);
            break;
        case '*':
            tok = NewToken(TokenType::MULT, ch, tok_index);
            break;
        case '/':
            tok = NewToken(TokenType::DIV, ch, tok_index);
            break;
        case '(':
            tok = NewToken(TokenType::LPAREN, ch, tok_index);
            break;
        case ')':
        tok = NewToken(TokenType::RPAREN, ch, tok_index);
        break;
        case '$':
            tok = NewToken(TokenType::EOL, ch, tok_index);
            break;
        // If none of the single-character lexemes (above) are matched,
        // check if its a symbol or number.
        // If its not even that, then its ILLEGAL
        default:
            if( IsLetter(ch)) {
                String8 sym = ReadSymbol();
                tok.literal = PushStringCopy(ctx.string_arena, sym);
                tok.type = TokenType::SYMBOL;
                tok.i = tok_index;
                return tok;
            } else if (IsNumber(ch)) {
                String8 num = ReadNumber();
                tok.literal = PushStringCopy(ctx.string_arena, num);
                tok.type = TokenType::NUM;
                tok.i = tok_index;
                return tok;
            } else {
                tok = NewToken(TokenType::ILLEGAL, ch, tok_index);
            }
    }

    ReadChar();
    return tok;
}

void Lexer::SkipWhitespace() {
    while (ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r') {
        ReadChar();
    }
}

void Lexer::ReadChar() {
    if (peek_position >= input.length) {
        ch = 0;
    } else {
        ch = input[peek_position];
    }
    position = peek_position;
    peek_position += 1;
}

char Lexer::PeekChar() const {
    if (peek_position >= input.length) {
        return 0;
    } else {
        return input[peek_position];
    }
}

String8 Lexer::ReadSymbol() {
    U64 start_pos = position;
    while (IsLetter(ch)) {
        ReadChar();
    }
    return input.substr({start_pos, position});
}

String8 Lexer::ReadNumber() {
    U64 start_pos = position;
    while (IsNumber(ch)) {
        ReadChar();
    }
    return input.substr({start_pos, position});
}

Token Lexer::NewToken(TokenType tokenType, char ch, U64 tok_index) {
    char* buf = ctx.string_arena.PushArrayNoZero<char>(1);
    *buf = ch;
    return Token{tokenType, String8(buf, 1), tok_index};
}

