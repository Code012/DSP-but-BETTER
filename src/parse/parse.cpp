/*  date = November 22nd 2025 11:03 PM */


#define TOKEN_BUF_SIZE 64
#define CODEPOINT_BUF_SIZE 64

namespace parse 
{ 

global U64 global_node_id = 1;


#if 0
////////////////////////////////
//- Something something

UnicodeDecode& 
CodepointPrefetchBuffer::operator[](U64 i)
{
	Assert(i < count);
	return decoded_codepoints[i];
}
const UnicodeDecode& 
CodepointPrefetchBuffer::operator[](U64 i) const
{
	Assert(i < count);
	return decoded_codepoints[i];
}
#endif

////////////////////////////////
//- Token Types

Token& 
TokenArray::operator[](U64 i) // with bounds checks as it should be
{
	Assert(i< count);
	return v[i];
}
const Token& 
TokenArray::operator[](U64 i) const
{
	Assert(i < count);
	return v[i];
}

/////////////////////////////////
//- Node Types

NodePool::NodePool()
	: arena(nullptr), first_free_node(nullptr)
{
	arena = ArenaAlloc();
}

/////////////////////////////////
//- Parser Types

Lexer::Lexer(String8 src)
	: src(src)
	, cursor(src.str)
{}

Parser::Parser(Arena* arena, String8 src)
	: node_pool{}
	, lexer{src}
	, token_cache{}
	, current_token{}
	, peek_token{}
	, msgs{}
{
	// initialise ring buffer
	token_cache.tokens.v = PushArray(arena, Token, 64); // ring buffer holds 64 tokens
	token_cache.tokens.count = 64;
	// set current and peek token
	NextToken(this);
}

////////////////////////////////
//- sb: Message Type Functions

internal void 
MsgListPush(Arena* arena, MsgList* msgs, Node* node, MsgKind kind, String8 string)
{
	Msg* msg = PushArray(arena, Msg, 1);
	msg->node = node;
	msg->kind = kind;
	msg->string = string;
	SLLQueuePush(msgs->first, msgs->last, msg);
	msgs->count += 1;
	msgs->worst_message_kind = Max(kind, msgs->worst_message_kind);
}


////////////////////////////////
//- sb: Node Type Functions

// sb: nil
internal B32 
NodeIsNil(Node* node)
{
	return (node == nullptr || node == 0 || node == &nil_node || node->kind == NodeKind::Nil);
}

// sb: node pool
internal Node* 
NodeAlloc(NodePool* node_pool)
{
	// grab top of free list
	Node* result = node_pool->first_free_node;
	if (result != nullptr)
	{
		// node_pool->first_free_node = node_pool->first_free_node->next;
		SLLStackPop(node_pool->first_free_node);
		MemoryZeroStruct(result);
	}

	// if free list was empty, push a new node onto the arena
	else
	{
		result = PushArray(node_pool->arena, Node, 1);
	}

	result->bin_left = result->bin_right = result->unary_child = result->nary_first = result->nary_next = &nil_node;
	return result;
}

internal void NodeRelease(NodePool* node_pool, Node* node)
{
	// push onto free list
	// node->next = node_pool->first_free_node;
	// node_pool->first_free_node = node;
	SLLStackPush(node_pool->first_free_node, node);
}

////////////////////////////////
//- sb: Text -> Tokens Functions
internal Token
NextTokenFromText(U8* byte_first, U8* one_past_last, U8*& cursor)
{
	TokenKind kind{};
	UnicodeDecode decoded{};
	U8* token_start = cursor;
	U8* token_opl = cursor;

	if (cursor < one_past_last)
	{
		// whitespace
		decoded = UTF8Decode(cursor, one_past_last - cursor);
		while (cursor < one_past_last && (CodepointIsSpace(decoded.codepoint)))
		{
			cursor += decoded.inc;
			decoded = UTF8Decode(cursor, one_past_last - cursor);	// current byte, bytes remaining
		}
		token_start = token_opl = cursor;

		switch (decoded.codepoint)
		{	
			// single character tokens
			case '+':
			{
				kind = TokenKind::Plus; 
				cursor += decoded.inc; 
			} break;
			case '(': 
			{
				kind = TokenKind::OpenParen; 
				cursor += decoded.inc;
			} break;
			case ')': 
			{ 
				kind = TokenKind::CloseParen; 
				cursor += decoded.inc;		
			} break;
			// Multiplication sign, Latin-1 Supplement
			case 215:		
			{
				kind = TokenKind::MultiplicationSign;
				cursor += decoded.inc;
			} break;
			// Division Sign, Latin-1 Supplement
			case 247: 		
			{
				kind = TokenKind::DivisionSign;
				cursor += decoded.inc;
			} break;
			// Minus Sign, Mathematical Operators
			case 8722: 		
			{
				kind = TokenKind::MinusSign;
				cursor += decoded.inc;
			} break;
			default:
			{
				// multi-character tokens
				// numerics
				if (CodepointIsDigit(decoded.codepoint))
				{
					kind = TokenKind::Numeric;
					while (cursor < one_past_last && ((CodepointIsDigit(decoded.codepoint) || decoded.codepoint == '.')))
					{
						cursor += decoded.inc;
						decoded = UTF8Decode(cursor, one_past_last - cursor);
					}
					token_opl = cursor;
				}

				// variables
				else if (CodepointIsAlpha(decoded.codepoint))
				{
					kind = TokenKind::Variable;
					while (cursor < one_past_last && (CodepointIsAlpha(decoded.codepoint)))
					{
						cursor += decoded.inc;
						decoded = UTF8Decode(cursor, one_past_last - cursor);
					}
					token_opl = cursor;
				}
			}	// default:
			break;
		} // switch (decoded.codepoint)

		token_opl = cursor;		// for single character tokens

		// check bad character
		if (kind == TokenKind::Nil && token_opl <= token_start)
		{
			kind = TokenKind::BadCharacter;
		}
	} // if (cursor < one_past_last)
	else
	{
		kind = TokenKind::EndOfInput;
	}

	U64 min = static_cast<U64>(token_start - byte_first); // ptrdiff_t -> U64 (signed to unsigned)
	U64 max = static_cast<U64>(token_opl - byte_first);
	Rng1U64 range = {min, max};

	return Token{range, kind};
}

#if 0
internal Token 
NextTokenFromText(U8* byte_first, U8* one_past_last, U8*& cursor)
{
	// scan string and produce token
	TokenKind kind = {};
	U8* token_start = cursor;
	U8* token_opl = cursor;

	if (cursor < one_past_last)
	{

		// whitespace
		while (cursor < one_past_last && (CharIsSpace(*cursor)))
			cursor++;
		token_start = token_opl = cursor;

		// single character tokens
		switch (*cursor)
		{
		case '+': kind = TokenKind::Plus; cursor++; break;
		case '-': kind = TokenKind::Minus; cursor++; break;
		case '*': kind = TokenKind::Star; cursor++; break;
		case '/': kind = TokenKind::Slash; cursor++; break;
		case '(': kind = TokenKind::OpenParen; cursor++; break;
		case ')': kind = TokenKind::CloseParen; cursor++; break;
		default:

		// multi-character tokens
			// numerics
			if (CharIsDigit(*cursor))
			{
				kind = TokenKind::Numeric;
				while (cursor < one_past_last && ((CharIsDigit(*cursor)) || *cursor == '.'))
					cursor++;
				token_opl = cursor;
			}

			// variables
			else if (CharIsAlpha(*cursor))
			{
				kind = TokenKind::Variable;
				while(cursor < one_past_last && (CharIsAlpha(*cursor)))
					cursor++;
				token_opl = cursor;
			}
			break;
		}

		token_opl = cursor;

		// check bad character and return
		if (kind == TokenKind::Nil && token_opl <= token_start)
		{
			kind = TokenKind::BadCharacter;
		}
	}
	else
	{
		kind = TokenKind::EndOfInput;
	}

	U64 min = static_cast<U64>(token_start - byte_first); // ptrdiff_t -> U64 (signed to unsigned)
	U64 max = static_cast<U64>(token_opl - byte_first);
	Rng1U64 range = {min, max};

	return Token{range, kind};
	
}
#endif

////////////////////////////////
//- sb: Text -> Tree Functions

// precedence table c++ doesnt have designated initialisers :(
global constexpr Precedence precedence_lookup[static_cast<U64>(TokenKind::MAX)] = {
	/* Nil */          Precedence::MIN,
    /* Plus */         Precedence::ADDITIVE,
    /* Minus */        Precedence::ADDITIVE,
    /* Star */         Precedence::MULTIPLICATIVE,
    /* Slash */        Precedence::MULTIPLICATIVE,
    /* ImplicitMult */ Precedence::IMPLICITMULT,
    /* Numeric */      Precedence::MIN,
    /* Variable */     Precedence::MIN,
    /* OpenParen */    Precedence::MIN,
    /* CloseParen */   Precedence::MIN,
    /* EndOfInput*/	   Precedence::MIN,
    /* BadCharacter */ Precedence::MIN
};

internal ParseResult 
ParseFromText(Parser* parser, String8 string)
{
	Node* expr = ParseExpression(parser, Precedence::MIN);

    // report unexpected trailing tokens
	// if (parser->current_token.kind != TokenKind::EndOfInput)

	ParseResult result = {expr, parser->msgs};
	return result;
}

internal Node* 
ParseExpression(Parser* parser, Precedence precedence)
{

	// parse NUD (lhs)
	Node* left = ParsePrefixExpression(parser);

	// parse all LED (rhs, attatch operators and rhs expressions)
	while (precedence < PeekPrecedence(parser))
	{
		NextToken(parser);

		left = ParseInfixExpression(parser, left);
	}

	return left;
}



internal Node* 
ParsePrefixExpression(Parser* p)
{
	Node* result = &nil_node;

	switch (p->current_token.kind)
	{
		case TokenKind::Numeric:
			result = ParseNumeric(p);
			break;
		case TokenKind::Variable:
			result = ParseVariable(p);
			break;
		case TokenKind::MinusSign:
			result = ParseUnary(p);
			break;
		case TokenKind::Plus:
			result = ParseUnary(p);
			break;
		case TokenKind::OpenParen:
			result = ParseGroup(p);
			break;
		default:
		{
			// log error to side channel
			String8 error_msg_start = Str8Lit("Unexpected token");

			U8* bad_char_start = p->lexer.src.str + p->current_token.range.min; 
			String8 error_msg_middle = Str8Range(bad_char_start, bad_char_start+1); // bad character string
			
			String8 error_msg_end = Str8Lit("at position");

			String8 msg = PushStr8F(log_arena, "%S '%S' %S %llu", error_msg_start, error_msg_middle, error_msg_end, p->current_token.range.min+1);
			MsgListPush(log_arena, &p->msgs, result, MsgKind::Error, msg);
			// create error node
			result = NodeAlloc(&p->node_pool);
			result->kind = NodeKind::ErrorMarker;
		}
		// implicit mult
	}

	return result;
}


internal Node* 
ParseNumeric(Parser* parser)
{
	ArenaTemp scratch = ScratchBegin(0, 0);

	U8* base = parser->lexer.src.str;

	U8* src_start = base + parser->current_token.range.min; 
	U8* src_opl = base + parser->current_token.range.max;
	
	String8 lexeme = Str8Range(src_start, src_opl);
	
	char* buf = PushArray(scratch.arena, char, lexeme.size);
	MemoryCopy(buf, lexeme.str, lexeme.size);
	buf[lexeme.size] = 0;
	F64 value = strtod((const char*)buf, nullptr);

	Rng1U64 num_range = parser->current_token.range;
	// NextToken(parser);

	Node* result = NodeAlloc(&parser->node_pool);
	result->bin_left = result->bin_right = result->unary_child = result->nary_first = result->nary_next = &nil_node;

	result->number = value;
	result->id = global_node_id++;
	result->kind = NodeKind::Number;
	result->num_operands = 0;
	result->original = num_range;
	result->modified = result->original;
	

	ScratchEnd(scratch);
	return result;	
}

internal Node* 
ParseVariable(Parser* parser)
{
	U8* base = parser->lexer.src.str;

	U8* src_start = base + parser->current_token.range.min; 
	U8* src_opl = base + parser->current_token.range.max;

	Rng1U64 var_range = parser->current_token.range;
	String8 var_name = Str8Range(src_start, src_opl);
	// NextToken(parser);

	Node* result = NodeAlloc(&parser->node_pool);
	result->bin_left = result->bin_right = result->unary_child = result->nary_first = result->nary_next = &nil_node;


	result->name = var_name;
	result->id = global_node_id++;
	result->kind = NodeKind::Variable;
	result->num_operands = 0;
	result->original = var_range;
	result->modified = result->original;

	return result;
}

internal Node* 
ParseUnary(Parser* p)
{	

	Rng1U64 op_range = p->current_token.range;

	B32 is_negative = (p->current_token.kind == TokenKind::MinusSign);

	NextToken(p);

	Node* result = NodeAlloc(&p->node_pool);

	result->bin_left = result->bin_right = result->unary_child = result->nary_first = result->nary_next = &nil_node;
	
	result->kind = NodeKind::UnaryOp;
	result->un_ops = is_negative ? UnOpKind::Negate: UnOpKind::Positive;
	result->id = global_node_id++;
	result->num_operands = 1;
	
	result->unary_child = ParseExpression(p, Precedence::UNARY);
	
	// compute ranges
	result->original.min = op_range.min;
	result->original.max = result->unary_child->original.max;
	result->modified = result->original;
	
	return result;
}

internal Node* 
ParseGroup(Parser* parser)
{
	NextToken(parser);
	Node* result = ParseExpression(parser, Precedence::MIN);
	if (parser->peek_token.kind == TokenKind::CloseParen)
	{
		NextToken(parser);
	}

	return result;
}

internal Node* 
ParseInfixExpression(Parser* parser, Node* left)
{
	Node* result = NodeAlloc(&parser->node_pool);

	result->num_operands = 2;

	switch (parser->current_token.kind)
	{
		case TokenKind::Plus: 	
			result->kind = NodeKind::NaryOp;  
			result->nary_ops = NaryOpKind::Plus;
			break;
		case TokenKind::MultiplicationSign:	
			result->kind = NodeKind::NaryOp;  
			result->nary_ops = NaryOpKind::Multiply;
			break;
		case TokenKind::MinusSign:	
			result->kind = NodeKind::BinaryOp;
			result->bin_ops = BinOpKind::Minus;
			break;
		case TokenKind::DivisionSign:	
			result->kind = NodeKind::BinaryOp;
			result->bin_ops = BinOpKind::Divide;
			break;
		default: 
			result->kind = NodeKind::ErrorMarker; break;
	}

	

	switch (result->kind)
	{
		case NodeKind::BinaryOp:
		{
			Precedence curr_prec = CurrentPrecedence(parser);
			NextToken(parser);
			result->bin_left = left;
			result->bin_right = ParseExpression(parser, curr_prec); // current precedence = left binding power
		} break;
		// At the parsing stage nary op behaves like binary op, the tree structure will be flattened for like-nary op nodes
		// For e.g. if a + node has a + node as a child, the child + will be removed and its operands will become the parent + node's operands
		case NodeKind::NaryOp:
		{
			Precedence curr_prec = CurrentPrecedence(parser);
			NextToken(parser);
			result->nary_first = left;
			result->nary_next = ParseExpression(parser, curr_prec); // current precedence = left binding power
		}break;
		case NodeKind::ErrorMarker:
		{
			// log error to side channel
			String8 error_msg_start = Str8Lit("Unexpected token");

			U8* bad_char_start = parser->lexer.src.str + parser->current_token.range.min; 
			String8 error_msg_middle = Str8Range(bad_char_start, bad_char_start+1); // bad character string
			
			String8 error_msg_end = Str8Lit("at position");

			String8 msg = PushStr8F(log_arena, "%S '%S' %S %llu", error_msg_start, error_msg_middle, error_msg_end, parser->current_token.range.min+1);
			MsgListPush(log_arena, &parser->msgs, result, MsgKind::Error, msg);

		} break;
		default: Assert(false); break; // shouldnt happen
	}

	return result;
}

/////////////////////////////////
//- sb: Parser Helpers

internal constexpr Precedence
PrecedenceFromKind(TokenKind tk)
{
	return precedence_lookup[static_cast<U64>(tk)];
}
internal constexpr Precedence
CurrentPrecedence(Parser* parser)
{
	return PrecedenceFromKind(parser->current_token.kind);
}
internal constexpr Precedence
PeekPrecedence(Parser* parser)
{
	return PrecedenceFromKind(parser->peek_token.kind);
}


#if 0
internal void
RefillCodepointPrefetchBuffer(Parser* p)
{
	CodepointPrefetchBuffer* codepoint_cache = &(p->codepoint_cache);

	codepoint_cache->current_index = 0;

	// populate
	for (U32 i = 0; i < CODEPOINT_BUF_SIZE; i++)
	{
		codepoint_cache[i] = NextCodepointFromText(p->lexer.src.str, p->lexer.src.str + p->lexer.src.size, p->lexer.cursor);
		if ((codepoint_cache[i] == U32Max))
		{
			//TODO: LOG ERROR
		}
	}
}
internal UnicodeDecode 
NextCodepoint(Parser* p)
{
	UnicodeDecode result{};	

	CodepointPrefetchBuffer* codepoint_cache = &(parser->codepoint_cache);

	// refill if all codepoints consumed
	if ((codepoint_cache->current_index == 0) || (codepoint_cache->current_index == CODEPOINT_BUF_SIZE))
		RefillCodepointPrefetchBuffer(parser);

	result = codepoint_cache[codepoint_cache->current_index++];
	return result;
}
#endif

internal void 
RefillTokenPrefetchBuffer(Parser* p)
{
	TokenPrefetchBuffer* token_cache = &(p->token_cache);

	token_cache->current_index = 0;

	// populate
	for (U32 i = 0; i < TOKEN_BUF_SIZE; i++)
	{
		token_cache->tokens[i] = NextTokenFromText(p->lexer.src.str, p->lexer.src.str + p->lexer.src.size, p->lexer.cursor);
		if ((token_cache->tokens[i].kind == TokenKind::EndOfInput))
			break;
	}
}

internal void 
NextToken(Parser* parser)
{
	if (parser->peek_token.kind != TokenKind::EndOfInput)
	{
		TokenPrefetchBuffer* token_cache = &(parser->token_cache);

		// refill if all 64 tokens consumed
		if ((token_cache->current_index == 0) || (token_cache->current_index == TOKEN_BUF_SIZE))
			RefillTokenPrefetchBuffer(parser);

		// consume current and peek
		parser->current_token = token_cache->tokens[token_cache->current_index]; 
		token_cache->current_index++;

		parser->peek_token = token_cache->tokens[token_cache->current_index];
	}
	else
	{
		parser->current_token = parser->peek_token;
	}
}

/////////////////////////////////
//- sb: Parser Debug Helpers (authored with AI assistance, GPT-5)

internal void DebugPrintParseResult(ParseResult result, String8 source)
{
	printf("\n=== Parse Result ==\n");

	if (result.msgs.count > 0)
	{
		printf("\nMessages (%llu):\n", result.msgs.count);
		for EachNode(msg, Msg, result.msgs.first)
		{
			char const* kind_str = "UNKNOWN";
			switch (msg->kind)
			{
				case MsgKind::Warning: kind_str = ColouriseYellow("WARNING"); break;
				case MsgKind::Error:   kind_str = ColouriseRed("ERROR"); break;
				default: break;
			}

			printf("  [%s] %.*s\n", kind_str, Str8Varg(msg->string));
		}
		printf("\n");
	}

	// print tree
	#if DEBUG_PARSER_TREE_VIEW
	printf("TREE:\n");
	DebugPrintNode(result.root, 1, "root");
	#else
	PrintNode(result.root, 1, "root");
	#endif
	printf("\n");
}

internal void PrintNode(Node* node, U32 depth, char const* label)
{
    (void)depth;
    (void)label;
    PrintExpr(node);
}


internal void PrintExpr(Node* node)
{
    if (NodeIsNil(node)) {
        printf("<nil>");
        return;
    }

    switch (node->kind)
    {
        case NodeKind::Number:
            printf("%g", node->number);
            break;

        case NodeKind::Variable:
            printf("%.*s", Str8Varg(node->name));
            break;

        case NodeKind::UnaryOp:
        {
            char const* op = (node->un_ops == UnOpKind::Negate) ? "-" : "+";
            printf("(");
            printf("%s", op);
            PrintExpr(node->unary_child);
            printf(")");
        } break;

        case NodeKind::BinaryOp:
        {
            char const* op = "?";
            switch (node->bin_ops) {
                case BinOpKind::Minus:  op = "-"; break;
                case BinOpKind::Divide: op = "/"; break;
                default: break;
            }

            printf("(");
            PrintExpr(node->bin_left);
            printf(" %s ", op);
            PrintExpr(node->bin_right);
            printf(")");
        } break;

        case NodeKind::NaryOp:
        {
            char const* op = "?";
            switch (node->nary_ops) {
                case NaryOpKind::Plus:     op = "+"; break;
                case NaryOpKind::Multiply: op = "*"; break;
                default: break;
            }

            printf("(");

            PrintExpr(node->nary_first);
            printf(" %s ", op);
            PrintExpr(node->nary_next);
            printf(")");
        } break;

        default:
            printf("<unknown>");
            break;
    }
}



internal void DebugPrintNode(Node* node, U32 depth, char const* label)
{
	// print indentation and optional label
	if (NodeIsNil(node))
	{
		if (label)
		{
			printf("%*s%s: <nil>\n", depth*2, "", label);
		}

		return;
	}

	if (label)
	{
		printf("%*s%s: ", depth*2, "", label);
	}
	else
	{
		printf("%*s", depth*2, "");
	}

	// print node info
	switch (node->kind)
	{
		case NodeKind::Nil:
		{
			printf("<nil node>\n");
		} break;
		case NodeKind::Number:
		{
			printf("Number( id=%llu, value=%.6f, range=[%llu,%llu) )\n",
					node->id, node->number, node->original.min, node->original.max);
		} break;
		case NodeKind::Variable:
		{
			printf("Variable( id=%llu, name=%.*s, range=[%llu,%llu) )\n",
					node->id, Str8Varg(node->name), node->original.min, node->original.max);
		} break;
		case NodeKind::UnaryOp:
		{
			char const* op_str = (node->un_ops == UnOpKind::Negate) ? "-" : "+";
			printf("UnaryOp( id=%llu, op='%s', range=[%llu,%llu) )\n",
					node->id, op_str, node->original.min, node->original.max);

			DebugPrintNode(node->unary_child, depth + 1, "child");
		} break;
		case NodeKind::BinaryOp:
		{	
			char const* op_str = "?";
			switch (node->bin_ops)
			{
				case BinOpKind::Minus:		op_str = "-"; break;
				case BinOpKind::Divide:		op_str = "/"; break;
				case BinOpKind::Fraction:	op_str = "frac"; break;
				case BinOpKind::Power:		op_str = "^"; break;
				default: break;
			}

			printf("BinaryOp( id=%llu, op='%s', range=[%llu,%llu) )\n",
					node->id, op_str, node->original.min, node->original.max);

			DebugPrintNode(node->bin_left, depth + 1, "left");
			DebugPrintNode(node->bin_right, depth + 1, "right");
		} break;
		case NodeKind::NaryOp:
		{	
			char const* op_str = "?";
			switch (node->nary_ops)
			{
				case NaryOpKind::Plus:		op_str = "+"; break;
				case NaryOpKind::Multiply:	op_str = "*"; break;
				default: break;
			}

			printf("NaryOp( id=%llu, op='%s', range=[%llu,%llu) )\n",
					node->id, op_str, node->original.min, node->original.max);

			DebugPrintNode(node->nary_first, depth + 1, "left");
			DebugPrintNode(node->nary_next, depth + 1, "right");
		} break;
		// case NodeKind::FunctionCall: break;
		case NodeKind::ErrorMarker:
		{
			printf("\x1b[31m ERROR( id=%llu, range=[%llu,%llu) ) \x1b[0m\n",
					node->id, node->original.min, node->original.max);			
		} break;
		default:
		{
			printf("Unknown( kind=%d )\n", (U32)node->kind);		
		} break;
	}



}

} // namespace parse
