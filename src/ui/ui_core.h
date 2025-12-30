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
	DeltaPicksSelectinSide 	= (1<<6),
};
IMPLEMENT_ENUM_CLASS_BITMASK(TextActionFlags, U32);

struct TextAction
{
	TextActionFlags flags;
	S64 delta;
	U32 codepoint;
};

internal TextAction TextActionFromEvent(OS::Event* event);
internal TextOp TextOpFromStateAndAction(Arena* arena, String8 string, TextEditState* state, TextAction* action);

internal void ApplyTextOp(Arena* arena, TextEditState* state, TextOp* op);

// helpers

internal S64 WordScan(String8 s, S64 cursor , S64 delta);
internal S64 CodePointScan(String8 s, S64 cursor , S64 delta);

// move this to base layer when done
// Returns how many BYTES the UTF-8 codepoint at `lead` occupies.
// `lead` must be the first byte of a UTF-8 codepoint.
internal S64 Utf8CodePointSize(U8 lead)
{
    // 0xxxxxxx
    // ASCII range (U+0000 – U+007F)
    // Single-byte codepoint
    if ((lead & 0x80) == 0x00)
    {
        return 1;
    }

    // 110xxxxx
    // Start of a 2-byte UTF-8 sequence
    if ((lead & 0xE0) == 0xC0)
    {
        return 2;
    }

    // 1110xxxx
    // Start of a 3-byte UTF-8 sequence
    if ((lead & 0xF0) == 0xE0)
    {
        return 3;
    }

    // 11110xxx
    // Start of a 4-byte UTF-8 sequence
    if ((lead & 0xF8) == 0xF0)
    {
        return 4;
    }

    // Any other pattern is invalid UTF-8:
    // - continuation byte (10xxxxxx)
    // - overlong / malformed lead
    //
    // Editors should fail softly:
    // treat it as a single byte to avoid infinite loops or crashes.
    return 1;
}

// Returns how many BYTES the previous UTF-8 codepoint occupies.
// `cursor` is a byte index *after* a codepoint.
internal inline S64 Utf8PrevCodePointSize(U8 *data, S64 cursor)
{
    S64 i = cursor - 1;

    // Walk backward over continuation bytes: 10xxxxxx
    while (i > 0 && (data[i] & 0xC0) == 0x80)
    {
        i--;
    }

    // cursor - i = number of bytes in the previous codepoint
    return cursor - i;
}

} // namespace UI

#endif // UI_CORE_H