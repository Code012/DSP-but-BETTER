//////////////////////////////
//- Build Options

#define BUILD_TITLE "tester"
#define BUILD_ENTRY_POINT_DEFINING_UNIT 1 // export to batch file
#define BUILD_COMMAND_LINE_INTERFACE 1 // export to batch file

//////////////////////////////
//- Parser Test Options

#define DEBUG_PARSER_TREE_VIEW 0


//////////////////////////////
//- Includes

//- foreign includes
#include <type_traits>

//- [h] root
#include "base/base_inc.h"
#include "os/os_inc.h"
#include "parse/parse_inc.h"

//- foreign includes
#define CLAY_IMPLEMENTATION
#include "third_party/clay/clay.h"
#include "third_party/clay/clay_renderer_raylib.h"
#include "tester/simpletest.h"
#include "tester/simpletest.cpp"

//- [cpp] root
#include "base/base_inc.cpp"
#include "os/os_inc.cpp"
#include "parse/parse_inc.cpp"

char const *groups[] = {
    "Bump",
    "Lexer",
    "Parser",
    // "Algebra"
};

U64 *DummyFunction2(Arena* escape)
{
    ArenaTemp ephemeral = ScratchBegin(&escape, 1);
    // scratch work doesn't outlive function
    U64* int_ptr = PushArray(ephemeral.arena, U64, 10);
    for (U64 i = 0; i < 10; i++ )
    {
        *int_ptr = max_u64;
    }
    // outlives function
    U64* result = PushArray(escape, U64, 1);
    *result = 100;
    ScratchEnd(ephemeral);

    return result;
}

U64 *DummyFunction(Arena* escape)
{
    ArenaTemp ephemeral = ScratchBegin(&escape, 1);
    U64* dummy = DummyFunction2(ephemeral.arena);
    // scratch work doesn't outlive function
    U64* int_ptr = PushArray(ephemeral.arena, U64, 10);
    for (U64 i = 0; i < 10; i++ )
    {
        *int_ptr = max_u64;
    }
    // outlives function
    U64* result = PushArray(escape, U64, 1);
    *result = 100;
    ScratchEnd(ephemeral);

    return result;
}

// Test basic arena functions, bugs will show up in usage code
DEFINE_TEST_G(ArenaBasicUsage, Bump)
{

    Arena *arena = ArenaAlloc(MiB(1));
    U64 *ptr = PushArray(arena, U64, 1);
    U64* res = DummyFunction(arena);
    *ptr = 5;
    TEST(arena != nullptr);
    TEST_EQ(arena->pos, 40);
    TEST_EQ(arena->commit_pos, 4096);
    TEST_EQ(arena->reserve_size, (MiB(64)));
    TEST(*ptr == 5);

    ArenaRelease(&arena);
    TEST(arena == nullptr);

    // *ptr += 1; // tested this, gives access violation


    {
        ArenaTemp ephemeral = ScratchBegin(0, 0); // could use raii here

        U64* int_ptr = PushArray(ephemeral.arena, U64, 2);
        int_ptr[0] = 5;
        int_ptr[1] = 6;

        TEST(int_ptr[0] == 5);
        TEST(int_ptr[1] == 6);

        U64* result = DummyFunction(ephemeral.arena);

        TEST(*result == 100);

        ScratchEnd(ephemeral);
    }
}

// old lexer, keeping it so i can compare with my new one, see if its better for LARGE input, purely for curiosity
#if 0
DEFINE_TEST_G(TestNextToken, Lexer) {

    ArenaTemp scratch = ScratchBegin(0, 0);

    String8 input = Str8Lit("(501341324 + 10234) * 32 + x / area");

    struct TestToken 
    {
        parse::TokenKind expected_kind;
        String8 expected_lexeme;
    };

    TestToken test_table[11];

    test_table[0] = TestToken{parse::TokenKind::OpenParen, Str8Lit("(")};
    test_table[1] = TestToken{parse::TokenKind::Numeric, Str8Lit("501341324")};
    test_table[2] = TestToken{parse::TokenKind::Plus, Str8Lit("+")};
    test_table[3] = TestToken{parse::TokenKind::Numeric, Str8Lit("10234")};
    test_table[4] = TestToken{parse::TokenKind::CloseParen, Str8Lit(")")};
    test_table[5] = TestToken{parse::TokenKind::Star, Str8Lit("*")};
    test_table[6] = TestToken{parse::TokenKind::Numeric, Str8Lit("32")};
    test_table[7] = TestToken{parse::TokenKind::Plus, Str8Lit("+")};
    test_table[8] = TestToken{parse::TokenKind::Variable, Str8Lit("x")};
    test_table[9] = TestToken{parse::TokenKind::Slash, Str8Lit("/")};
    test_table[10] = TestToken{parse::TokenKind::Variable, Str8Lit("area")};

    parse::TokeniseResult res = parse::TokeniseFromText(scratch.arena, input);
    parse::TokenArray tokens = res.tokens;
    for (U64 i = 0; i < tokens.count; i++)
    {
        TEST(tokens[i].kind == test_table[i].expected_kind);
        // match lexeme
        String8 lexeme = Str8Range(input.str + tokens[i].range.min, input.str+tokens[i].range.max);
        TEST(Str8Match(lexeme, test_table[i].expected_lexeme, StringMatchFlags::None) != 0);
    }

    ScratchEnd(scratch);
} 
#endif  

DEFINE_TEST_G(TestNextToken, Parser) {

    

    ArenaTemp scratch = ScratchBegin(0,0);
    
    String8 tests[] = { // gpt for test cases
        Str8Lit("2+2"),
        Str8Lit("−5"),
        Str8Lit("+7"),
        Str8Lit("−(3)"),
        Str8Lit("+(4)"),
        Str8Lit("1+−2"),
        Str8Lit("1−−2"),
        Str8Lit("(−1)+(+2)"),
        Str8Lit("3×(2+1)"),
        Str8Lit("(3+2)×4"),
        Str8Lit("10÷(5−3)"),
        Str8Lit("−(1+2)×3"),
        Str8Lit("((1+2)×3)−4"),
        Str8Lit("1+2×3−4÷2"),
        Str8Lit("((1))"),
        Str8Lit("−((3+5)×2)"),
        };

    for (U32 i = 0; i < ArrayCount(tests); i++)
    {
        String8 input = tests[i];
        parse::Parser parser{scratch.arena, input}; 
        parse::ParseResult result = parse::ParseFromText(scratch.arena, &parser, input); // pass arena for ring buffer scratch work
        printf("\n==============================================================");
        printf("\nInput: %.*s\n", Str8Varg(input));

        parse::DebugPrintParseResult(result, input);
        
        if (parser.msgs.count > 0)
        {
            for EachNode(msg, parse::Msg, parser.msgs.first)
            {
                PrintRed((U32)msg->string.size, msg->string.str);
            }
        }
    }

    ScratchEnd(scratch);
}

DEFINE_TEST_G(TestNextToken, Algebra) {
    ArenaTemp scratch = ScratchBegin(0, 0);

    String8 input = Str8Lit("2+2");
    parse::Parser parser{scratch.arena, input};
    parse::ParseResult result = parse::ParseFromText(scratch.arena, &parser, input);
    printf("\nInput: %.*s\n", Str8Varg(input));
    if (parser.msgs.count > 0)
    {
        for EachNode(msg, parse::Msg, parser.msgs.first)
        {
            PrintRed((U32)msg->string.size, msg->string.str);
        }
    }

    // result.root = algebra::AutomaticSimplify(result.root);

    parse::PrintExpr(result.root);

    ScratchEnd(scratch);
}


internal void 
EntryPoint(U64 argument_count, char** arguments) 
{

    bool pass = true;

    for (auto group : groups) 
    {
        pass &= TestFixture::ExecuteTestGroup(group, TestFixture::Verbose);
    }
}


#if 0
/*  date = December 12th 2025 06:16 PM */
// Single-line text edit with event list architecture
// Based on Ryan Fleury's text editing approach

#include "third_party/raylib/include/raylib.h"
#include <string>
#include <vector>
#include <algorithm>

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Data Structures

namespace OS
{
    enum class Key : uint32_t
    {
        Null,
        A, B, C, D, E, F, G, H, I, J, K, L, M,
        N, O, P, Q, R, S, T, U, V, W, X, Y, Z,
        Backspace, Delete, Left, Right, Home, End,
        Enter, Tab, Space,
        COUNT
    };

    enum class Modifiers : uint32_t
    {
        None    = 0,
        Ctrl    = (1<<0),
        Shift   = (1<<1),
        Alt     = (1<<2),
    };

    inline Modifiers operator|(Modifiers a, Modifiers b) {
        return (Modifiers)((uint32_t)a | (uint32_t)b);
    }
    inline Modifiers operator&(Modifiers a, Modifiers b) {
        return (Modifiers)((uint32_t)a & (uint32_t)b);
    }
    inline bool HasFlag(Modifiers flags, Modifiers flag) {
        return ((uint32_t)flags & (uint32_t)flag) != 0;
    }

    enum class EventKind : uint32_t
    {
        Null,
        Press,
        Release,
        Text,
    };

    struct Event
    {
        EventKind kind;
        Key key;
        Modifiers modifiers;
        uint32_t codepoint;
        bool consumed; // Track if event has been "eaten"
    };

    struct EventList
    {
        std::vector<Event> events;
        
        void Add(const Event& event) {
            events.push_back(event);
        }
        
        void Clear() {
            events.clear();
        }
        
        size_t Count() const {
            size_t count = 0;
            for (const auto& e : events) {
                if (!e.consumed) count++;
            }
            return count;
        }
    };

    // Convert Raylib key to OS::Key
    Key MapRaylibKey(int key)
    {
        if (key >= KEY_A && key <= KEY_Z)
            return (Key)((int)Key::A + (key - KEY_A));
        
        switch (key)
        {
            case KEY_BACKSPACE: return Key::Backspace;
            case KEY_DELETE:    return Key::Delete;
            case KEY_LEFT:      return Key::Left;
            case KEY_RIGHT:     return Key::Right;
            case KEY_HOME:      return Key::Home;
            case KEY_END:       return Key::End;
            case KEY_ENTER:     return Key::Enter;
            case KEY_TAB:       return Key::Tab;
            case KEY_SPACE:     return Key::Space;
            default:            return Key::Null;
        }
    }

    Modifiers GetCurrentModifiers()
    {
        Modifiers mods = Modifiers::None;
        if (IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL))
            mods = mods | Modifiers::Ctrl;
        if (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT))
            mods = mods | Modifiers::Shift;
        if (IsKeyDown(KEY_LEFT_ALT) || IsKeyDown(KEY_RIGHT_ALT))
            mods = mods | Modifiers::Alt;
        return mods;
    }

    // Collect events from Raylib for this frame
    // Note: Unlike Windows PeekMessage, this is synchronous per-frame
    EventList GetEvents()
    {
        EventList list;
        Modifiers mods = GetCurrentModifiers();

        // Collect key press events
        int key = GetKeyPressed();
        while (key != 0)
        {
            Event event = {};
            event.kind = EventKind::Press;
            event.key = MapRaylibKey(key);
            event.modifiers = mods;
            event.consumed = false;

            if (event.key != Key::Null) {
                list.Add(event);
            }

            key = GetKeyPressed();
        }

        // Collect text input events
        int codepoint = GetCharPressed();
        while (codepoint != 0)
        {
            Event event = {};
            event.kind = EventKind::Text;
            event.key = Key::Null;
            event.codepoint = codepoint;
            event.modifiers = mods;
            event.consumed = false;

            list.Add(event);
            codepoint = GetCharPressed();
        }

        return list;
    }

    // Mark an event as consumed (Ryan's "eat" pattern)
    void EatEvent(Event* event)
    {
        if (event) {
            event->consumed = true;
            event->kind = EventKind::Null;
        }
    }
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// UI Text Editing

namespace UI
{
    struct Rng1S64
    {
        int64_t min;
        int64_t max;
    };

    struct TextOp
    {
        Rng1S64 range;          // range within edited string to replace
        std::string replace;    // text to replace range with
        std::string copy;       // text to copy to clipboard
        int64_t new_cursor;     // cursor point, after op
        int64_t new_mark;       // mark point, after op
    };

    struct TextEditState
    {
        int64_t cursor;
        int64_t mark;
        std::string text;
    };

    TextOp TextOpFromEvent(const TextEditState& state, const OS::Event& event, const std::string& text)
    {
        TextOp op = {};
        op.new_cursor = state.cursor;
        op.new_mark = state.mark;

        int64_t cursor = state.cursor;
        int64_t mark = state.mark;
        int64_t text_len = (int64_t)text.length();

        // Selection range
        int64_t sel_min = std::min(cursor, mark);
        int64_t sel_max = std::max(cursor, mark);
        bool has_selection = (sel_min != sel_max);

        if (event.kind == OS::EventKind::Press)
        {
            bool ctrl = OS::HasFlag(event.modifiers, OS::Modifiers::Ctrl);
            bool shift = OS::HasFlag(event.modifiers, OS::Modifiers::Shift);

            switch (event.key)
            {
                // Movement
                case OS::Key::Left:
                {
                    if (ctrl) {
                        // Word left
                        int64_t new_pos = cursor;
                        while (new_pos > 0 && text[new_pos - 1] == ' ') new_pos--;
                        while (new_pos > 0 && text[new_pos - 1] != ' ') new_pos--;
                        op.new_cursor = new_pos;
                    } else {
                        op.new_cursor = std::max((int64_t)0, cursor - 1);
                    }
                    op.new_mark = shift ? mark : op.new_cursor;
                } break;

                case OS::Key::Right:
                {
                    if (ctrl) {
                        // Word right
                        int64_t new_pos = cursor;
                        while (new_pos < text_len && text[new_pos] != ' ') new_pos++;
                        while (new_pos < text_len && text[new_pos] == ' ') new_pos++;
                        op.new_cursor = new_pos;
                    } else {
                        op.new_cursor = std::min(text_len, cursor + 1);
                    }
                    op.new_mark = shift ? mark : op.new_cursor;
                } break;

                case OS::Key::Home:
                {
                    op.new_cursor = 0;
                    op.new_mark = shift ? mark : 0;
                } break;

                case OS::Key::End:
                {
                    op.new_cursor = text_len;
                    op.new_mark = shift ? mark : text_len;
                } break;

                // Deletion
                case OS::Key::Backspace:
                {
                    if (has_selection) {
                        op.range = {sel_min, sel_max};
                        op.new_cursor = op.new_mark = sel_min;
                    } else if (cursor > 0) {
                        op.range = {cursor - 1, cursor};
                        op.new_cursor = op.new_mark = cursor - 1;
                    }
                } break;

                case OS::Key::Delete:
                {
                    if (has_selection) {
                        op.range = {sel_min, sel_max};
                        op.new_cursor = op.new_mark = sel_min;
                    } else if (cursor < text_len) {
                        op.range = {cursor, cursor + 1};
                        op.new_cursor = op.new_mark = cursor;
                    }
                } break;

                // Clipboard operations
                case OS::Key::C:
                {
                    if (ctrl && has_selection) {
                        op.copy = text.substr(sel_min, sel_max - sel_min);
                    }
                } break;

                case OS::Key::X:
                {
                    if (ctrl && has_selection) {
                        op.copy = text.substr(sel_min, sel_max - sel_min);
                        op.range = {sel_min, sel_max};
                        op.new_cursor = op.new_mark = sel_min;
                    }
                } break;

                case OS::Key::V:
                {
                    if (ctrl) {
                        const char* clipboard = GetClipboardText();
                        if (clipboard) {
                            if (has_selection) {
                                op.range = {sel_min, sel_max};
                                op.new_cursor = op.new_mark = sel_min;
                            } else {
                                op.range = {cursor, cursor};
                            }
                            op.replace = clipboard;
                            op.new_cursor = op.new_mark = op.range.min + (int64_t)op.replace.length();
                        }
                    }
                } break;

                // Select all
                case OS::Key::A:
                {
                    if (ctrl) {
                        op.new_cursor = text_len;
                        op.new_mark = 0;
                    }
                } break;

                default: break;
            }
        }
        else if (event.kind == OS::EventKind::Text)
        {
            // Text insertion
            if (has_selection) {
                op.range = {sel_min, sel_max};
                op.new_cursor = op.new_mark = sel_min;
            } else {
                op.range = {cursor, cursor};
            }
            
            // Convert UTF-32 codepoint to UTF-8
            char utf8[5] = {0};
            int len = 0;
            if (event.codepoint < 0x80) {
                utf8[0] = (char)event.codepoint;
                len = 1;
            } else if (event.codepoint < 0x800) {
                utf8[0] = 0xC0 | (event.codepoint >> 6);
                utf8[1] = 0x80 | (event.codepoint & 0x3F);
                len = 2;
            } else if (event.codepoint < 0x10000) {
                utf8[0] = 0xE0 | (event.codepoint >> 12);
                utf8[1] = 0x80 | ((event.codepoint >> 6) & 0x3F);
                utf8[2] = 0x80 | (event.codepoint & 0x3F);
                len = 3;
            } else {
                utf8[0] = 0xF0 | (event.codepoint >> 18);
                utf8[1] = 0x80 | ((event.codepoint >> 12) & 0x3F);
                utf8[2] = 0x80 | ((event.codepoint >> 6) & 0x3F);
                utf8[3] = 0x80 | (event.codepoint & 0x3F);
                len = 4;
            }
            
            op.replace = std::string(utf8, len);
            op.new_cursor = op.new_mark = op.range.min + len;
        }

        return op;
    }

    void ApplyTextOp(TextEditState& state, const TextOp& op)
    {
        // Copy to clipboard if needed
        if (!op.copy.empty()) {
            SetClipboardText(op.copy.c_str());
        }

        // Apply text replacement
        if (op.range.min != op.range.max || !op.replace.empty()) {
            state.text.erase(op.range.min, op.range.max - op.range.min);
            state.text.insert(op.range.min, op.replace);
        }

        // Update cursor and mark
        state.cursor = op.new_cursor;
        state.mark = op.new_mark;
    }

    // Process events for a text editor (can consume events)
    void TextEditConsumeEvents(TextEditState& state, OS::EventList& events)
    {
        for (auto& event : events.events)
        {
            if (event.consumed) continue;

            TextOp op = TextOpFromEvent(state, event, state.text);
            
            // Check if this event produced any operation
            bool did_something = 
                (op.range.min != op.range.max) ||     // Text was replaced
                (!op.replace.empty()) ||               // Text was inserted
                (!op.copy.empty()) ||                  // Text was copied
                (op.new_cursor != state.cursor) ||    // Cursor moved
                (op.new_mark != state.mark);          // Mark moved

            if (did_something) {
                ApplyTextOp(state, op);
                OS::EatEvent(&event); // Consume the event
            }
        }
    }
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Rendering

void DrawTextEdit(const UI::TextEditState& state, Rectangle box)
{
    // Background
    DrawRectangleRec(box, WHITE);
    DrawRectangleLinesEx(box, 2, BLACK);

    // Text
    const char* text = state.text.c_str();
    int text_x = (int)box.x + 10;
    int text_y = (int)box.y + (int)((box.height - 30) / 2);
    DrawText(text, text_x, text_y, 30, BLACK);

    // Selection
    int64_t sel_min = std::min(state.cursor, state.mark);
    int64_t sel_max = std::max(state.cursor, state.mark);
    if (sel_min != sel_max) {
        std::string before_sel = state.text.substr(0, sel_min);
        std::string selection = state.text.substr(sel_min, sel_max - sel_min);
        int sel_x = text_x + MeasureText(before_sel.c_str(), 30);
        int sel_width = MeasureText(selection.c_str(), 30);
        DrawRectangle(sel_x, text_y - 2, sel_width, 34, ColorAlpha(BLUE, 0.3f));
    }

    // Cursor (blinking)
    if (fmod(GetTime(), 1.0) < 0.5) {
        std::string before_cursor = state.text.substr(0, state.cursor);
        int cursor_x = text_x + MeasureText(before_cursor.c_str(), 30);
        DrawRectangle(cursor_x, text_y - 2, 2, 34, RED);
    }
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Main

internal void 
EntryPoint(U64 argument_count, char** arguments) 
{
    InitWindow(800, 600, "Event List Text Edit");
    SetTargetFPS(60);

    UI::TextEditState edit_state = {};
    edit_state.text = "Hello, World!";
    edit_state.cursor = edit_state.text.length();
    edit_state.mark = edit_state.cursor;

    while (!WindowShouldClose())
    {
        // Get events for this frame (Ryan Fleury pattern)
        OS::EventList events = OS::GetEvents();

        // Process text editing (consumes relevant events)
        UI::TextEditConsumeEvents(edit_state, events);

        // Other systems could process remaining unconsumed events here
        // For example: button clicks, menu navigation, etc.
        
        // Debug: show unconsumed events
        size_t unconsumed = events.Count();

        // Draw
        BeginDrawing();
        ClearBackground(DARKGRAY);

        DrawText("Event List Architecture Demo", 50, 50, 30, WHITE);
        DrawText("Using Ryan Fleury's event pattern with Raylib", 50, 90, 20, LIGHTGRAY);
        
        DrawText("Features:", 50, 140, 20, YELLOW);
        DrawText("- Arrow keys, Home/End navigation", 50, 170, 18, LIGHTGRAY);
        DrawText("- Ctrl+Arrow for word jump", 50, 195, 18, LIGHTGRAY);
        DrawText("- Shift to select, Ctrl+A select all", 50, 220, 18, LIGHTGRAY);
        DrawText("- Ctrl+C/X/V for copy/cut/paste", 50, 245, 18, LIGHTGRAY);

        // Text editor
        Rectangle text_box = {50, 300, 700, 50};
        DrawTextEdit(edit_state, text_box);

        // Debug info
        DrawText(TextFormat("Unconsumed events: %zu", unconsumed), 50, 380, 20, GREEN);
        DrawText(TextFormat("Cursor: %lld  Mark: %lld", edit_state.cursor, edit_state.mark), 50, 410, 20, GREEN);

        EndDrawing();
    }

    CloseWindow();
}
#endif
