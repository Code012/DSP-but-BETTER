/*  date = November 22nd 2025 11:03 PM */


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
//- sb: Token Type Functions

internal Token* 
TokenPush(Arena* arena, TokenKind kind, Rng1U64 rng)
{
	Token* t = PushArray(arena, Token, 1);
	t->kind = kind;
	t->range = rng;

	return t;
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

////////////////////////////////
//- sb: Text -> Tokens Functions

internal TokeniseResult TokeniseFromText(Arena* arena, String8 string)
{
	ArenaTemp scratch = ScratchBegin(&arena, 1);

	TokenArray token_array = zero_struct;
	token_array.v = PushArray(arena, Token, 300);	// allocating for 300 tokens up front (~7KB), because i dont want to implement a chunked list for something so simple.
	MsgList msgs = zero_struct;

	U8* byte_first = string.str;
	U8* one_past_last = byte_first + string.size;
	U8* cursor = byte_first;


	// scan string and produce tokens
	while (cursor < one_past_last)
	{
		TokenKind kind = {};
		U8* token_start = cursor;
		U8* token_opl = cursor;

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

			else
			{
				// error 
				// could potentially add mismatched parenthesis, etc 
				kind = TokenKind::BadCharacter;
				U64 min = static_cast<U64>(token_start - byte_first); // ptrdiff_t -> U64 (signed to unsigned)
				U64 max = static_cast<U64>(token_opl - byte_first);
				Rng1U64 src_range = {min, max};
				Node* error = PushNode(arena, NodeKind::ErrorMarker, 0.0, Str8Lit(""), BinOpKind::Nil, UnOpKind::Nil, src_range, {0, 0});
				String8 error_string = Str8Lit("Unexpected character.");
				MsgListPush(arena, &msgs, error, MsgKind::Error, error_string);
				cursor++;
			}
			break;
		}

		token_opl = cursor;

		// push token to token array
		if (kind != TokenKind::Nil && token_opl > token_start)
		{
			U64 min = static_cast<U64>(token_start - byte_first); // ptrdiff_t -> U64 (signed to unsigned)
			U64 max = static_cast<U64>(token_opl - byte_first);
			Rng1U64 range = {min, max};
			Token* t = TokenPush(arena, kind, range);
			token_array[token_array.count++] = *t;
		}
	}

	TokeniseResult result = zero_struct;
	result.tokens = token_array;
	result.msgs = msgs;

	ScratchEnd(scratch);
	return result;
}

} // namespace parse
