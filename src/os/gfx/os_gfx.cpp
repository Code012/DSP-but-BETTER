/*  date = December 12th 2025 06:39 PM */

namespace OS
{

////////////////////////////////////////////////////////////////
//- helpers
internal B32 IsValidCodePoint(U32 codepoint)
{
    B32 is_valid = false;
    if (codepoint <= 43 && codepoint >= 40)         // (, ), *, +
        is_valid = true;
    else if (codepoint >= 45 && codepoint <= 57)    // -, ., /, 0123456789
        is_valid = true;
    else if (codepoint == 61 || codepoint == 32)    // =, SPACE
        is_valid = true;
    else if (codepoint <= 122 && codepoint >= 97)   // a-z 
        is_valid = true;
    else if (codepoint == 13)                       // CARRIAGE RETURN
        is_valid = true;

    return is_valid;
    
}

internal U32 CanonicalUnicodeSymbolFromAscii(U32 ascii_codepoint)
{
    U32 unicode_codepoint = ascii_codepoint;

    switch (ascii_codepoint)
    {
        case 42: {                      // asterik
        unicode_codepoint = 215;        // Muliplication Symbol
        } break;
        case 45: {                      // hyphen
            unicode_codepoint = 8722;   // Subtraction Symbol
        } break;
        case 47: {                      // forward slash
            unicode_codepoint = 247;    // Division Symbol
        } break;
    }

    return unicode_codepoint;
}

internal Modifier GetModifiers(void)
{
    Modifier mods = (Modifier)0;
    if (IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL))
    {
        mods = mods | Modifier::Ctrl;
        OutputDebugStringA("Ctrl pressed\n");
    }
    if (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT))
    {
        mods = mods | Modifier::Shift;
        OutputDebugStringA("Shift pressed\n");
    }
    if (IsKeyDown(KEY_LEFT_ALT) || IsKeyDown(KEY_RIGHT_ALT))
    {
        mods = mods | Modifier::Alt;
        OutputDebugStringA("Alt pressed\n");
    }
    return mods;
}

bool IsModifierKey(Key k)
{
    return k == Key::Ctrl || k == Key::Shift || k == Key::Alt;
}


internal EventList* GetKeyboardEvents(Arena* arena)
{

    EventList* list = PushArray(arena, EventList, 1);
    Modifier mods = GetModifiers();

    // 1. Collect key press events
    int key = GetKeyPressed();
    while (key != 0)
    {
        Key os_key = MapRaylibKey(key);
        
        if (os_key != Key::Null && !(IsModifierKey(os_key)))
        {
            Event* event = PushArray(arena, Event, 1);
            event->kind = EventKind::Press;
            event->key = os_key;
            event->modifiers = mods;
            event->codepoint = 0;
            
            DLLPushBack(list->first, list->last, event);
            list->count += 1;
        }
        key = GetKeyPressed();
    }

    int repeat_keys[] = {
        KEY_BACKSPACE,
        KEY_DELETE,
        KEY_LEFT,
        KEY_RIGHT
    };

    // 2. Collect repeat key press events from the repeat_keys list (the ones we want to check for)
    for (int i = 0; i < ArrayCount(repeat_keys); i++)
    {
        key = repeat_keys[i];
        if (IsKeyPressedRepeat(key))  // this returns true only on frames where the OS generated key-repeat events for that particular key (raylib queues events remmeber)
        {
            Key os_key = MapRaylibKey(key);
            if (os_key != Key::Null && !(IsModifierKey(os_key)))
            {
                Event* event = PushArray(arena, Event, 1);
                event->kind = EventKind::Press;
                event->key = os_key;
                event->modifiers = mods;
                event->codepoint = 0;
                
                DLLPushBack(list->first, list->last, event);
                list->count += 1;
            }
        }
    }

    // 3. Collect key release events
    // int key = GetKeyReleased();

    // 4. Collect text input events
    key = GetCharPressed();   // raylib handles Shift+[a-z]
    while (key != 0 && IsValidCodePoint(key)) // restricted to ascii subset only, see todo.txt for reason (mostly to make my life easier)
    {

        U32 unicode_symbol = CanonicalUnicodeSymbolFromAscii(key);

        
        Event* event = PushArray(arena, Event, 1);
        event->kind = EventKind::Text;
        event->key = Key::Null;
        event->modifiers = mods;
        event->codepoint = unicode_symbol;
        
        DLLPushBack(list->first, list->last, event);
        list->count += 1;
        
        key = GetCharPressed();
    }

    return list;
}

internal void EatEvent(EventList* events, Event* event)
{
    if (event && events)
    {
        DLLRemove(events->first, events->last, event);
        events->count -= 1;
        event->kind = EventKind::Null;
    }
}

internal Key MapRaylibKey(int key)
{
    if (key >= KEY_A && key <= KEY_Z)
        return (Key)((int)Key::A + (key - KEY_A));
    
    if (key >= KEY_ZERO && key <= KEY_NINE)
        return (Key)((int)Key::_0 + (key - KEY_ZERO));
    
    // Function keys
    if (key >= KEY_F1 && key <= KEY_F12)
        return (Key)((int)Key::F1 + (key - KEY_F1));
    
    switch (key)
    {
        case KEY_ESCAPE:        return Key::Esc;
        case KEY_GRAVE:         return Key::GraveAccent;
        case KEY_MINUS:         return Key::Minus;
        case KEY_EQUAL:         return Key::Equal;
        case KEY_BACKSPACE:     return Key::Backspace;
        case KEY_DELETE:        return Key::Delete;
        case KEY_TAB:           return Key::Tab;
        case KEY_SPACE:         return Key::Space;
        case KEY_ENTER:         return Key::Enter;
        case KEY_LEFT_CONTROL:
        case KEY_RIGHT_CONTROL: return Key::Ctrl;
        case KEY_LEFT_SHIFT:
        case KEY_RIGHT_SHIFT:   return Key::Shift;
        case KEY_LEFT_ALT:
        case KEY_RIGHT_ALT:     return Key::Alt;
        case KEY_UP:            return Key::Up;
        case KEY_LEFT:          return Key::Left;
        case KEY_DOWN:          return Key::Down;
        case KEY_RIGHT:         return Key::Right;
        case KEY_PAGE_UP:       return Key::PageUp;
        case KEY_PAGE_DOWN:     return Key::PageDown;
        case KEY_HOME:          return Key::Home;
        case KEY_END:           return Key::End;
        case KEY_SLASH:         return Key::ForwardSlash;
        case KEY_PERIOD:        return Key::Period;
        case KEY_COMMA:         return Key::Comma;
        case KEY_APOSTROPHE:    return Key::Quote;
        case KEY_LEFT_BRACKET:  return Key::LeftBracket;
        case KEY_RIGHT_BRACKET: return Key::RightBracket;
        case KEY_INSERT:        return Key::Insert;
        case KEY_SEMICOLON:     return Key::Semicolon;
        default:                return Key::Null;
	}
}

}	// namespace OS