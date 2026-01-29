/*  date = December 12th 2025 03:30 PM */

namespace UI
{

////////////////////////////////////////////////////////////////
//- Text Editing

internal TextAction 
TextActionFromEvent(OS::Event* event)
{
	TextAction action = zero_struct;
	action.codepoint = event->codepoint;

	if (HasFlag(event->modifiers, OS::Modifier::Ctrl))
	{
		action.flags |= TextActionFlags::WordScan;
	}
	if (HasFlag(event->modifiers, OS::Modifier::Shift))
	{
		action.flags |= TextActionFlags::KeepMark;
	}

	switch (event->key)
	{
		default:{}break;
		case OS::Key::Right: 	
		{
			action.delta = +1;
			action.flags |= TextActionFlags::DeltaPicksSelectinSide;
		}break;
		case OS::Key::Left:  	
		{
			action.delta = -1;
			action.flags |= TextActionFlags::DeltaPicksSelectinSide;
		}break;
		case OS::Key::Home:		{action.delta = S64Max;}break; // enfore post-navigation, cursors will be clamped to the range of valid points for a given string
		case OS::Key::End:		{action.delta = S64Min;}break; // enfore post-navigation, cursors will be clamped to the range of valid points for a given string
		case OS::Key::Backspace:
		{
			action.delta = -1;
			action.flags |= TextActionFlags::Delete | TextActionFlags::ZeroDeltaWithSelection;
		}break;
		case OS::Key::Esc:
		{
			action.delta = +1;
			action.flags |= TextActionFlags::Delete | TextActionFlags::ZeroDeltaWithSelection;
		}break;
		case OS::Key::C: if(HasFlag(event->modifiers, OS::Modifier::Ctrl))
		{
			action.flags |= TextActionFlags::Copy;
		}break;
		case OS::Key::X: if(HasFlag(event->modifiers, OS::Modifier::Ctrl))
		{
			action.flags |= TextActionFlags::Copy|TextActionFlags::Delete;
		}break;
		case OS::Key::V: if(HasFlag(event->modifiers, OS::Modifier::Ctrl))
		{
			action.flags |= TextActionFlags::Paste;
		}break;
	}

	return action;
}

internal TextOp 
TextOpFromStateAndAction(Arena* arena, String8 string, TextEditState* state, TextAction* action)
{
	TextOp op = zero_struct;

	// defaults... assume no substantial changes
	op.new_cursor = state->cursor;
	op.new_mark = state->mark;
	op.replace_string = Str8Lit("");
	op.range = Rng1S64{0, 0};
	// op.range = Rng1S64{Min(state->cursor, state->mark),
						// Max(state->cursor, state->mark)};

	// --- navigation ----------------------------------------------

	// high-level delta -> low-level delta
	S64 ll_delta = 0;
	{
		if (HasFlag(action->flags, TextActionFlags::WordScan))
		{
			ll_delta = WordScan(string, state->cursor, action->delta);
		}
		else
		{
			ll_delta = CodePointScan(string, state->cursor, action->delta);
		}
	}
	 
	// zero-delta on selection
	if (state->cursor != state->mark && 
		HasFlag(action->flags, TextActionFlags::ZeroDeltaWithSelection))
	{
		ll_delta = 0;
	}

	// pick selection side
	if (state->cursor != state->mark &&
		HasFlag(action->flags, TextActionFlags::DeltaPicksSelectinSide))
	{
		ll_delta = 0;
		if (action->delta > 0)
		{
			op.new_cursor = Max(state->cursor, state->mark);
		}
		else if (action->delta < 0)
		{
			op.new_cursor = Min(state->cursor, state->mark);
		}
	}

	// apply delta
	op.new_cursor = state->cursor + ll_delta;

	

	// --- post-navigation ------------------------------------------

	// copy
	if (HasFlag(action->flags, TextActionFlags::Copy))
	{
		op.copy_string = Str8Substr(string, Rng1U64{static_cast<U64>(op.new_cursor), static_cast<U64>(op.new_mark)});
	}

	// delete
	if (HasFlag(action->flags, TextActionFlags::Delete))
	{
		op.range = Rng1S64{op.new_cursor, op.new_mark};
		op.new_cursor = op.new_mark = op.range.min;
	}

	// insert / paste - mutually exclusive
	if (action->codepoint != 0)
	{
		op.replace_string = PushString8FromCodepoint(arena, action->codepoint);
		S64 insert_at = Min(op.new_cursor, op.new_mark);

		op.range = {		// selection range
			insert_at,
			Max(op.new_cursor, op.new_mark)
		};

		S64 new_pos = insert_at + (S64)op.replace_string.size;
		op.new_cursor = op.new_mark = new_pos;
	}
	else if (HasFlag(action->flags, TextActionFlags::Paste))
	{
		// TODO
	}

	// keep mark (has to happen after deletion)
	if (!HasFlag(action->flags, TextActionFlags::KeepMark))
	{
		op.new_mark = op.new_cursor;
	}


	// Don't go over limit
	op.new_cursor = ClampTop(op.new_cursor, App::app_state->input_box_limit-1);
	op.new_mark = ClampTop(op.new_mark, App::app_state->input_box_limit-1);

	return op;
}

internal void 
ApplyTextOp(Arena* arena, TextEditState* state, TextOp* op)
{
    // Copy to clipboard if needed
    // if (op->copy_string.size > 0)
    // {
    //     ArenaTemp temp = ScratchBegin(&arena, 1);
    //     String8 null_terminated = PushStr8Copy(temp.arena, op->copy_string);
    //     PushArray(temp.arena, U8, 1); // null terminator
    //     SetClipboardText((const char*)null_terminated.str);
    //     ScratchEnd(temp);
    // }
    
    // Apply text replacement
    // Invariant: there is always a selection
    // String8 old_text = state->text;
    // S64 new_size = old_text.size - (op->range.max - op->range.min) + op->replace_string.size;
    // if (new_size < App::app_state->input_box_limit)															// not  tracking grapheme clusters because idk how rn, and this is easier
	// {
	//     if (op->range.min != op->range.max || op->replace_string.size > 0)
	//     {
	        
	//         // U8* new_str = PushArray(arena, U8, new_size);
	//         U8* new_str = state->text.str;
	        
	//         // Copy before range
	//         MemoryCopy(new_str, old_text.str, op->range.min);
	        
	//         // Copy replacement
	//         MemoryCopy(new_str + op->range.min, op->replace_string.str, op->replace_string.size);
	        
	//         // Copy after range
	//         MemoryCopy(new_str + op->range.min + op->replace_string.size,
	//                    old_text.str + op->range.max,
	//                    old_text.size - op->range.max);
	        
	//         state->text = Str8(new_str, new_size);
	//     }
	// }

	ArenaTemp scratch = ScratchBegin(0, 0);

	
    // Update cursor and mark
    state->cursor = op->new_cursor;
    state->mark = op->new_mark;

    // TODO(sb): set clipboard text

    // replace string
    // String8 modified_string = UI::PushStringReplaceRange(scratch.arena, op->replace_string,
    													// Rng1U64{op->replace.min, op->replace.max},
    											// )
    // TODO(sb): clean this code up
    // Invariant: There is always a selection
    String8 result = zero_struct;
    String8 edit_string = state->text;

    Rng1U64 range = Rng1U64{static_cast<U64>(op->range.min), static_cast<U64>(op->range.max)};
    range.min = ClampTop(range.min, edit_string.size);
    range.max = ClampTop(range.max, edit_string.size);
    range = Rng1U64{range.min, range.max};
    U64 result_size = edit_string.size - Dim1U64(range) + op->replace_string.size;

    if (result_size < App::app_state->input_box_limit)	// not tracking grapheme clusters because idk how rn, and this is easier TODO(sb): figure it out if you have time
	{
	    result.str = PushArray(scratch.arena, U8, result_size);
	    result.size = edit_string.size;
	    MemoryCopy(result.str, edit_string.str, edit_string.size);
	    
	    String8 before_range = Prefix8(result, range.min);
	    String8 after_range = Str8Skip(result, range.max);
	    edit_string.size = result_size;

	    if (before_range.size != 0)
	    {
	    	MemoryCopy(edit_string.str, before_range.str, before_range.size);
	    }
	    if (op->replace_string.size != 0)
	    {
	    	MemoryCopy(edit_string.str + range.min, op->replace_string.str, op->replace_string.size);
	    }
	    if (after_range.size != 0)
	    {
	    	MemoryCopy(edit_string.str + range.min + op->replace_string.size, after_range.str, after_range.size);
	    }
	}

    state->text = edit_string;


    ScratchEnd(scratch);
}


internal S64 
WordScan(String8 s, S64 cursor , S64 delta)
{
	return 0;
}

internal S64
CodePointScan(String8 s, S64 cursor , S64 delta)
{	
	S64 start = cursor;

	// move right
	if (delta > 0)
	{
		while (delta-- > 0 && cursor < s.size)
			cursor += Utf8CodePointSize(s.str[cursor]);
	}
	else if (delta < 0)
	{
		while (delta++ < 0 && cursor > 0)
			cursor -= Utf8PrevCodePointSize(s.str, cursor);
	}
	return cursor - start;
}

}	// namespace UI