/*  date = November 22nd 2025 11:03 PM */


#define TOKEN_BUF_SIZE 64

namespace parse 
{ 

global U64 global_node_id = 1;


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
	, tokens_rb{}
	, current_token{}
	, peek_token{}
	, msgs{}
{
	// initialise ring buffer
	tokens_rb.tokens.v = PushArray(arena, Token, 64); // ring buffer holds 64 tokens
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
	return (node == nullptr || node == &nil_node || node->kind == NodeKind::Nil);
}

// sb: tree building
internal Node* 
PushNode(Arena* arena, NodeKind kind, double value, String8 name, BinOpKind bk, UnOpKind uk, Rng1U64 src, Rng1U64 mod)
{
	Node* node = PushArray(arena, Node, 1);
	// error nodes id: 0, valid nodes incremental
	// node->id = (kind == NodeKind::ErrorMarker ? 0 : ++global_node_id);

	node->bin_left = node->bin_right = node->unary_child = node->func_arguments = &nil_node;
	node->kind = kind;
	node->original = src;
	node->modified = mod;

	if (kind != NodeKind::ErrorMarker)
		node->id = global_node_id++;
	else
		node->id = 0;

	if (kind != NodeKind::ErrorMarker)
	{
		switch (kind)
		{
		case NodeKind::Number:       
			node->number = value; 
			break;

		case NodeKind::Variable:
		case NodeKind::FunctionCall: 
			node->name = name; 
			break;

		case NodeKind::BinaryOp:     
			node->bin_ops = bk; 
			break;

		case NodeKind::UnaryOp: 	 
			node->un_ops = uk; 
			break;

		case NodeKind::ErrorMarker:
			node->id = 0;

		default:					 
			break;
		}
	}
	
	return node;
}

// sb: node pool
internal Node* NodeAlloc(NodePool* node_pool)
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

internal Token NextTokenFromText(U8* byte_first, U8* one_past_last, U8*& cursor)
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
ParseFromText(Arena* arena, Parser* parser, String8 string)
{
	ArenaTemp scratch = ScratchBegin(arena, 1);
	MsgList msgs = zero_struct;

	Node* expr = ParseExpression(parser, Precedence::MIN);

    // report unexpected trailing tokens
	// if (parser->current_token.kind != TokenKind::EndOfInput)

	ParseResult result = {expr, msgs};
	ScratchEnd(scratch);
	return result;
}

internal Node* ParseExpression(Parser* parser, Precedence precedence)
{

	// parse NUD (lhs)
	Node* left = ParsePrefixExpression(parser);

	// parse all LED (rhs, attatch operators and rhs expressions)
	// while (precedence < PeekPrecedence(parser))
	// {
	// 	NextToken(parser);

	// 	left = ParseInfixExpression(parser, left);
	// }

	return left;
}



internal Node* ParsePrefixExpression(Parser* p)
{
	Node* result;

	switch (p->current_token.kind)
	{
		case TokenKind::Numeric:
			result = ParseNumeric(p);
			break;
		case TokenKind::Variable:
			result = ParseVariable(p);
			break;
		case TokenKind::Minus:
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
			

			}
		// implicit mult
	}

	return result;
}

internal Node* ParseNumeric(Parser* parser)
{
	U8* base = parser->lexer.src.str;

	U8* src_start = base + parser->current_token.range.min; 
	U8* src_opl = base + parser->current_token.range.max;
	
	String8 lexeme = Str8Range(src_start, src_opl);
	
	F64 value = strtod((const char*)lexeme.str, nullptr);
	Rng1U64 num_range = parser->current_token.range;
	NextToken(parser);

	Node* result = NodeAlloc(&parser->node_pool);
	result->bin_left = result->bin_right = result->unary_child = result->func_arguments = &nil_node;

	result->number = value;
	result->id = global_node_id++;
	result->kind = NodeKind::Number;
	result->original = num_range;
	result->modified = result->original;
	
	return result;	
}

internal Node* ParseVariable(Parser* parser)
{
	U8* base = parser->lexer.src.str;

	U8* src_start = base + parser->current_token.range.min; 
	U8* src_opl = base + parser->current_token.range.max;

	Rng1U64 var_range = parser->current_token.range;
	String8 var_name = Str8Range(src_start, src_opl);
	NextToken(parser);

	Node* result = NodeAlloc(&parser->node_pool);
	result->bin_left = result->bin_right = result->unary_child = result->func_arguments = &nil_node;


	result->name = var_name;
	result->id = global_node_id++;
	result->kind = NodeKind::Variable;
	result->original = var_range;
	result->modified = result->original;

	return result;
}

internal Node* ParseUnary(Parser* p)
{	

	Rng1U64 op_range = p->current_token.range;

	B32 is_negative = (p->current_token.kind == TokenKind::Minus);

	NextToken(p);

	Node* result = NodeAlloc(&p->node_pool);

	result->bin_left = result->bin_right = result->unary_child = result->func_arguments = &nil_node;
	
	result->kind = NodeKind::UnaryOp;
	result->un_ops = is_negative ? UnOpKind::Negate: UnOpKind::Positive;
	result->id = global_node_id++;
	
	result->unary_child = ParseExpression(p, Precedence::UNARY);
	
	// compute ranges
	result->original.min = op_range.min;
	result->original.max = result->unary_child->original.max;
	result->modified = result->original;
	
	return result;
}

internal Node* ParseGroup(Parser* parser)
{
	NextToken(parser);
	Node* result = ParseExpression(parser, Precedence::MIN);
	if (parser->current_token.kind == TokenKind::CloseParen)
	{
		NextToken(parser);
	}

	return result;
}

// internal Node* ParseInfixExpression(Parser* parser, Node* left)
// {

// }

/////////////////////////////////
//- sb: Parser Helpers

internal constexpr Precedence
PrecedenceFromKind(TokenKind tk)
{
	return precedence_lookup[static_cast<U64>(tk)];
}
internal constexpr Precedence
PeekPrecedence(Parser* parser)
{
	return PrecedenceFromKind(parser->peek_token.kind);
}

internal void RefillRingBuffer(Parser* p)
{
	TokenRingBuffer* ring_buf = &(p->tokens_rb);

	ring_buf->head = 0;
	ring_buf->tail = 0;

	// populate
	for (U32 i = 0; i < TOKEN_BUF_SIZE; i++)
	{
		ring_buf->tokens[i] = NextTokenFromText(p->lexer.src.str, p->lexer.src.str + p->lexer.src.size, p->lexer.cursor);
	}

	ring_buf->tail = TOKEN_BUF_SIZE;
}

internal void NextToken(Parser* parser)
{
	if (parser->peek_token.kind != TokenKind::EndOfInput)
	{
		TokenRingBuffer* ring_buf = &(parser->tokens_rb);

		// refill if all 64 tokens consumed
		if (ring_buf->head == ring_buf->tail)
		{
			RefillRingBuffer(parser);
		}

		// consume current
		parser->current_token = ring_buf->tokens[ring_buf->head]; 
		ring_buf->head++;

		// ensure peek is valid
		if (ring_buf->head == ring_buf->tail)
		{
			RefillRingBuffer(parser);
		}

		parser->peek_token = ring_buf->tokens[ring_buf->head];
	}
}

} // namespace parse
