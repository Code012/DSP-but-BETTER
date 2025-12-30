/*  date = December 12th 2025 06:39 PM */ 

#ifndef OS_GFX_H
#define OS_GFX_H

namespace OS
{
////////////////////////////////
//

enum class Key
{
Null,
Esc,
F1,
F2,
F3,
F4,
F5,
F6,
F7,
F8,
F9,
F10,
F11,
F12,
F13,
F14,
F15,
F16,
F17,
F18,
F19,
F20,
F21,
F22,
F23,
F24,
GraveAccent,
_0,
_1,
_2,
_3,
_4,
_5,
_6,
_7,
_8,
_9,
Minus,
Equal,
Backspace,
Delete,
Tab,
A,
B,
C,
D,
E,
F,
G,
H,
I,
J,
K,
L,
M,
N,
O,
P,
Q,
R,
S,
T,
U,
V,
W,
X,
Y,
Z,
Space,
Enter,
Ctrl,
Shift,
Alt,
Up,
Left,
Down,
Right,
PageUp,
PageDown,
Home,
End,
ForwardSlash,
Period,
Comma,
Quote,
LeftBracket,
RightBracket,
Insert,
MouseLeft,
MouseMiddle,
MouseRight,
Semicolon,
COUNT,
};

////////////////////////////////
//- Events

enum class EventKind
{
	Null,
	WindowClose,
	Press,
	Release,
	Text,
	COUNT,
};

enum class Modifier : U32
{
	Ctrl 	= (1<<0),
	Shift 	= (1<<1),
	Alt 	= (1<<2),
};
IMPLEMENT_ENUM_CLASS_BITMASK(Modifier, U32)

struct Event
{
	Event* next;
	Event* prev;
	EventKind kind;
	Modifier modifiers;
	B32 is_modified;
	Key key;
	U32 codepoint;
};

struct EventList
{
	Event* first;
	Event* last;
	U64 count;
};

////////////////////////////////////////////////////////////////
//- helpers
internal B32 IsValidCodePoint(S64 key);

////////////////////////////////////////////////////////////////
//- @os_per_backend Events

internal Modifier GetModifiers(void);
internal EventList* GetKeyboardEvents(Arena* arena);
internal void EatEvent(EventList* events, Event* event);
internal Key MapRaylibKey(int key);

} // namespace OS

#endif // OS_GFX_H