/*  date = February 11th 2026 04:47 PM  */

////////////////////////////////
// Based on Ryan Fleury's Metadesk patterns
// Following NeGate's advice for a lex-ahead ring buffer
// - We lex tokens in batches (64 at a time) into a fixed-size buffer
// - N-reads N-writes as opposed to 1-read 1-write. Lexer writes 64 tokens, parser reads 64 tokens
// - Each buffer slot is reused repeatedly, so writes and subsequent reads
// happen on the same memory locations, keeping data (more likely) hot in L1 cache 
// - By limiting the working set to a small buffer, we reduce cache misses,
// TLB misses, and memory traffic compared to pre-tokenising the entire file
// - Memory usage scaled with parse depth, not file size, making this
// approach cache-friendly for large files with typical nesting.

// ^ TODO: do a benchmark comparing this approach with pre-lexing for large inputs (small inputs doesnt matter if we do either way).

// TODO: 
// 1. Attach source ranges to AST nodes for GUI highlighting later.
// 2. Add node IDs so transformed nodes can be tracker across steps.
// 3. Store both original + updated ranges for step-by-step expression changes.
// 4. idk what im talking about, experiment until you come to a reasonable solution.
// 5. See if double is not needed, if a fixed point would be better

#ifndef EXPR_PARSE_H
#define EXPR_PARSE_H

namespace expr {

////////////////////////////////
//- Side-channel Error Messages

enum class MsgKind : U32
{
	Null,
	Warning, // for cases that technically work, but is the user sure they want to write it this way
	Error,
};

struct Msg
{
	Msg *next;
	struct Node *node;  		// map error to node (and node has src ranges to highlight)
	MsgKind kind;
	String8 string;
};

struct MsgList
{
	Msg *first;
	Msg *last;
	U64 count;
	MsgKind worst_message_kind;
};


////////////////////////////////
//- Token Types

enum class TokenKind : U32
{
	// sb: base kind info
	Nil           = 0,
    Plus,
    MinusSign,//Hyphen,
    MultiplicationSign,//Star,
    DivisionSign,///Slash,
    ImplicitMult,
    Numeric,
    Variable,
    OpenParen,
    CloseParen,

    EndOfInput,
    // sb: error info
    BadCharacter,

    MAX,
};

// TODO: is there a need to classify token groups? Perhaps based on their associativity?

struct Token
{
	Rng1U64 range;
	TokenKind kind;
};

struct TokenArray
{
	Token *v;
	U64 count;

	// with bounds checks as it should be
	Token& operator[](U64 i);
	const Token& operator[](U64 i) const;
};

struct TokenPrefetchBuffer
{
	TokenArray tokens;
	U64 current_index;
};

/////////////////////////////////
//- Operator Types

enum class NaryOpKind: U32
{
	Nil=0,
	Plus,
	Multiply,
};

enum class BinOpKind : U32
{
	Nil=0,
	Minus, 
	Divide,
	Power,
};

enum class UnOpKind : U32
{
	Nil=0,
	Negate,
	Positive,
};

/////////////////////////////////
//- Node Types

enum class NodeKind : U32
{
	Nil,
	Number,			// Leaf: numeric literal
	Variable,		// Leaf: identifier
	UnaryOp,		// Internal: -{expression}
	BinaryOp,		// Internal: -, /, FRACTION, ^
	NaryOp,	        // Internal: *, +
	FunctionCall,	// Internal: sin(x), sqrt(x)
	ErrorMarker,	// not sure if i need this, but experimenting
	COUNT,
};

enum class NodeFlags: U32
{
	// _B represent "base classification"
	// _S represent "specific classification"
	// _B enumerators will always be set, so that its _S enumerators can be checked
	Nil 		= (1u << 0),
	Undefined 	= (1u << 1),
	//- Base Enumerations
	Integer_B 	= (1u << 2),
	Symbol_B	= (1u << 3),
	FracOp_B	= (1u << 4),		// define these terms (BinaryOp)
	PowOp_B   	= (1u << 5),		// (UnaryOp)
	ProdOp_B	= (1u << 6),		// 
	SumOp_B		= (1u << 7),
	QuotOp_B	= (1u << 8),
	FactOp_B	= (1u << 9),
	DiffOp_B 	= (1u << 10),

	//- Specific Enumerations
	// ProdOp_B
	UnaryProdOp_S = (1u << 11),		// *x -> x [Definition 3.16]
	// SumOp_B
	UnarySumOp_S 	= (1u << 12),	// +x -> x [Definition 3.16]
	// DiffOp_B
	UnaryDiffOp_S 	= (1u << 13),	// Each unary difference op is replaced by the product [Definition 3.12.1]
	BinaryDiffOp_S	= (1u << 14),	// Each binary difference op is replaced by the sum [Definition 3.12.2]
	COUNT,
};
IMPLEMENT_ENUM_CLASS_BITMASK(NodeFlags, U32);



// Note(sb): For future me. Yes this looks really weird. Yes it was more readable with a tagged union, however I need the pointers to be first for the nil node to work. 
// p.s. the pointers are prefixed with the type of node its for (for some readability so its not too bad)
struct Node
{
	// sb: tree links
	Node* unary_child;
	Node* bin_left;
	Node* bin_right;
	Node* nary_first;
	Node* nary_next;
	// Node* func_arguments;

	// sb: free list link
	Node* next;

	// sb: payload
	union 					// Tidbit(sb): Recently learnt from this (https://eel.is/c++draft/class#union.general-2) that type punning with this union will be UB, but I'm fine because of the invariant that the union value will be selected in accordance with the node kind. Type punning with memcpy is far better, or std::bitcast from c++20
	{
		double number;
		String8 name;		// interchangeable for variable name or function name
		NaryOpKind nary_ops;
		BinOpKind bin_ops;
		UnOpKind un_ops;
	};

	// sb: node info
	U64 id;
	U64 num_operands;			// useful for nary operands
	NodeKind kind;
	NodeFlags flags;			// useful for simplification stage

	// sb: source code info
	Rng1U64 original;
	Rng1U64 modified;

};

struct NodePool
{
	NONCOPYABLE_NONMOVABLE(NodePool)
	NodePool();

	Arena* arena;
	Node* first_free_node;
};


////////////////////////////////
///- Parser Types

enum class Precedence : U32
{
	MIN=0,

	TERM=1,
	ADDITIVE=10,
	MULTIPLICATIVE=20,
	IMPLICITMULT=30,
	UNARY=40,
	EXPONENTIAL=50,

	MAX = 100, // make it U32 max
};

struct Lexer
{
	DEFAULT_CTOR_DTOR(Lexer)
	Lexer(String8 src);

	String8 src;
	U8* cursor;
};

struct Parser
{
	DEFAULT_CTOR_DTOR(Parser)
	NONCOPYABLE_NONMOVABLE(Parser)
	Parser(Arena* arena, String8 src);

	// memory
	NodePool node_pool;	

	Lexer lexer;

	// CodepointPrefetchBuffer codepoint_cache;

	// tokens
	TokenPrefetchBuffer token_cache;
	Token current_token;
	Token peek_token;

	// errors
	MsgList msgs;
};

////////////////////////////////
//- sb: Text -> Tree Types

struct ParseResult
{
	Node* root;
	MsgList msgs;
};

////////////////////////////////
//- sb: Globals (https://www.rfleury.com/p/the-easiest-way-to-handle-errors)

global read_only Node nil_node = 
{
	&nil_node,
	&nil_node,
	&nil_node,
	&nil_node,
	&nil_node,
	&nil_node,
};

////////////////////////////////
//- sb: Message Type Functions

internal void MsgListPush(Arena* arena, MsgList* msgs, Node* node, MsgKind kind, String8 string);

////////////////////////////////
//- sb: Node Type Functions

// sb: nil
internal B32 NodeIsNil(Node* node);
#define NodeSetNil(p) ((p) = &nil_node)

// sb: node pool
internal Node* NodeAlloc(NodePool* node_pool);
internal void NodeRelease(NodePool* node_pool, Node* node);

////////////////////////////////
//- sb: Text -> Tokens Functions

// internal UnicodeDecode NextCodepointFromText(U8* byte_first, U8* one_past_last, U8*& cursor);
internal Token NextTokenFromText(U8* byte_first, U8* one_past_last, U8*& cursor);

////////////////////////////////
//- sb: Text -> Tree Functions

internal ParseResult ParseFromText(Parser* parser, String8 string);

internal Node* ParseExpression(Parser* parser, Precedence precedence);

internal Node* ParsePrefixExpression(Parser* parser);
internal Node* ParseNumeric(Parser* parser);
internal Node* ParseVariable(Parser* parser);
internal Node* ParseUnary(Parser* parser);
internal Node* ParseGroup(Parser* parser);

internal Node* ParseInfixExpression(Parser* parser, Node* left);

/////////////////////////////////
//- sb: Parser Helpers

internal constexpr Precedence PrecedenceFromKind(TokenKind tk);
internal constexpr Precedence CurrentPrecedence(Parser* parser);
internal constexpr Precedence PeekPrecedence(Parser* parser);

// internal void RefillCodepointPrefetchBuffer(Parser* p);
// internal void NextCodepoint(Parser* p);

internal void RefillTokenPrefetchBuffer(Parser* p);
internal void NextToken(Parser* parser);

internal B32 IsInteger(Node* expr);
internal NodeFlags ComputeNodeFlags(Node* node);

/////////////////////////////////
//- sb: Parser Debug Helpers (authored with AI assistance, GPT-5)

internal void DebugPrintParseResult(ParseResult result, String8 source);
internal void DebugPrintNode(Node* node, U32 depth = 0, char const* label = nullptr);
internal void PrintNode(Node* node, U32 depth, char const* label);
internal void PrintExpr(Node* node);

}	// namespace expr

#endif	// EXPR_PARSE_H