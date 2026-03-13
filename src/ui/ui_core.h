/*  date = December 12th 2025 03:30 PM */ 

#ifndef UI_CORE_H
#define UI_CORE_H




namespace UI
{


////////////////////////////////////////////////////////////////
//- Text Editing

struct TextOp
{
	Rng1S64 range; 	// range within edited string to replace
	String8 replace_string; // text to replace range with
	String8 copy_string; 	// text to copy to clipboard
	S64 new_cursor;			// cursor point, after op
	S64 new_mark; 			// mark point, after op
};

struct TextEditState
{
	String8 text;
	S64 cursor;
	S64 mark;
    F32 scroll_offset_x;
};

enum class TextActionFlags : U32
{
	WordScan 				= (1<<0),
	KeepMark 				= (1<<1),
	Delete 					= (1<<2),
	Copy    				= (1<<3),
	Paste                   = (1<<4),
	ZeroDeltaWithSelection 	= (1<<5),
	DeltaPicksSelectionSide = (1<<6),
};
IMPLEMENT_ENUM_CLASS_BITMASK(TextActionFlags, U32);

struct TextAction
{
	TextActionFlags flags;
	S64 delta;
	U32 codepoint;
};

struct NodeBox
{
    expr::Node* node;
    Rng2F32 rect;
    U32 step_index;
};

// Text Edit
internal TextAction TextActionFromEvent(OS::Event* event);
internal TextOp TextOpFromStateAndAction(Arena* arena, String8 string, TextEditState* state, TextAction* action);

internal void ReplaceTextRange(String8& buffer, Rng1S64 replace_range, String8 replace_string, S64 new_buffer_size);
internal void ApplyTextOp(TextEditState* state, TextOp* op);


// helpers

internal S64 WordScan(String8 s, S64 cursor , S64 delta);
internal S64 CodePointScan(String8 s, S64 cursor , S64 delta);



} // namespace UI

#endif // UI_CORE_H