/*  date = November 22nd 2025 06:13 PM */ 

////////////////////////////////
// Based on Ryan Fleury's Metadesk patterns

// Note(sb): For real world input where there can be thousands of tokens, use a chunked list (dynamic list) and then flatten into array at the end

// TODO: 
// 1. Attach source ranges to AST nodes for GUI highlighting later.
// 2. Add node IDs so transformed nodes can be tracker across steps.
// 3. Store both original + updated ranges for step-by-step expression changes.
// 4. idk what im talking about, experiment until you come to a reasonable solution.
// 5. See if double is not needed, if a fixed point would be better

#ifndef PARSE_HPP
#define PARSE_HPP

namespace parse 
{

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
    Plus      	  = (1u << 0),
    Minus     	  = (1u << 1),
    Star	  	  = (1u << 2),
    Slash	      = (1u << 3),
    UnaryMinus	  = (1u << 4),
    ImplicitMult  = (1u << 5),
    Numeric  	  = (1u << 6),
    Variable  	  = (1u << 7),
    OpenParen	  = (1u << 8),
    CloseParen    = (1u << 9),
    EndOfInput    = (1u << 11),

    // sb: error info
    BadCharacter  = (1u << 10),
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

/////////////////////////////////
//- Operator Types

enum class BinOpKind : U32
{
	Nil=0,
	Plus,
	Minus, 
	Multiply,
	Divide,
	Fraction,
	Power,
};

enum class UnOpKind : U32
{
	Nil=0,
	Negate,
};

/////////////////////////////////
//- Node Types

enum class NodeKind : U32
{
	Nil,
	Number,			// Leaf: numeric literal
	Variable,		// Leaf: identifier
	BinaryOp,		// Internal: +, -, *, /, FRACTION, ^
	UnaryOp,		// Internal: - 
	FunctionCall,	// Internal: sin(x), sqrt(x)
	ErrorMarker,	// not sure if i need this, but experimenting
	COUNT,
};


// Note(sb): For future me. Yes this looks really weird. Yes it was more readable with a tagged union, however I need the pointers to be first for the nil node to work. 
// p.s. the pointers are prefixed with the type of node its for (for some readability so its not too bad)
struct Node
{
	// sb: tree links
	Node* bin_left;
	Node* bin_right;
	Node* unary_child;
	Node* func_arguments;

	// sb: payload
	union
	{
		double number;
		String8 name;		// interchangeable for variable name or function name
		BinOpKind bin_ops;
		UnOpKind un_ops;
	};

	// sb: node info
	U64 id;
	NodeKind kind;

	// sb: source code info
	Rng1U64 original;
	Rng1U64 modified;

};

////////////////////////////////
//- sb: Text -> Tokens Types

struct TokeniseResult
{
	TokenArray tokens;
	MsgList msgs;
};

////////////////////////////////
//- sb: Tokens -> Tree Types

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
};

////////////////////////////////
//- sb: Message Type Functions

internal void MsgListPush(Arena* arena, MsgList* msgs, Node* node, MsgKind kind, String8 string);

////////////////////////////////
//- sb: Token Type Functions

internal Token* TokenPush(Arena* arena, TokenKind kind, Rng1U64 rng);

////////////////////////////////
//- sb: Node Type Functions

// sb: nil
internal B32 NodeIsNil(Node* node);
#define NodeSetNil(p) ((p) = &nil_node)

// sb: tree building
internal Node* PushNode(Arena* arena, NodeKind kind, double value, String8 name, BinOpKind bk, UnOpKind uk, Rng1U64 src, Rng1U64 mod);


////////////////////////////////
//- sb: Text -> Tokens Functions

internal TokeniseResult TokeniseFromText(Arena* arena, String8 string);

////////////////////////////////
//- sb: Tokens -> Tree Functions

// internal ParseResult ParseFromTe


////////////////////
} // namespace parse 
#endif // PARSE_HPP