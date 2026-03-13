/*  date = December 09th 2025 06:15 PM */ 

internal void RenderTextCursor(Arena* arena, UI::TextEditState* state, Font* fonts);

#define INPUT_TEXT_OFFSET 1024	// fits up to 200 glyphs assuming 4-byte UTF-8 codepoints (worst case)
#define CLUSTER_LIMIT 200 		// not yet used


namespace g
{
	read_only global int codepoints[] = {
		// Unicode Block: Basic Latin, U+0000 - U+007F
		#if !defined(BUILD_DEBUG)
		0x2B,	// +
		0X2E,	// .
		0X30,	// 0
		0X31,	// 1
		0X32,	// 2
		0X33,	// 3
		0X34,	// 4
		0X35,	// 5
		0X36,	// 6
		0X37,	// 7
		0X38,	// 8
		0X39,	// 9
		0X3D,	// =
		0X41,	// A
		0X42,	// B
		0X43,	// C
		0X44,	// D
		0X45,	// E
		0X46,	// F
		0X47,	// G
		0X48,	// H
		0X49,	// I
		0X4A,	// J
		0X4B,	// K
		0X4C,	// L
		0X4D,	// M
		0X4E,	// N
		0X4F,	// O
		0X50,	// P
		0X51,	// Q
		0X52,	// R
		0X53,	// S
		0X54,	// T
		0X55,	// U
		0X56,	// V
		0X57,	// W
		0X58,	// X
		0X59,	// Y
		0X5A,	// Z
		0x61,	// a
	    0x62,	// b
	    0x63,	// c
	    0x64,	// d
	    0x65,	// e
	    0x66,	// f
	    0x67,	// g
	    0x68,	// h
	    0x69,	// i
	    0x6A,	// j
	    0x6B,	// k
	    0x6C,	// l
	    0x6D,	// m
	    0x6E,	// n
	    0x6F,	// o
	    0x70,	// p
	    0x71,	// q
	    0x72,	// r
	    0x73,	// s
	    0x74,	// t
	    0x75,	// u
	    0x76,	// v
	    0x77,	// w
	    0x78,	// x
	    0x79,	// y
	    0x7A,	// z
	#endif
	    // Unicode Block: Latin-1 Supplement, U+0080 - U+00FF
	    0XD7,	// ×, Multiplication Sign
	    0XF7,	// ÷, Division Symbol
 
	    // Unicode Block: Mathematical Operators, U+2200 - U+22FF
	    0X2212	// −, Minus Sign

	};
}	//namespace global


namespace App
{

struct ScrollBarData
{
	Clay_Vector2 click_origin;
	Clay_Vector2 position_origin;
	B32 mouse_down;
};



struct State
{	
	// Permanent arenas
	Arena* clay_arena;
	Arena* string_arena;		// 1024 bytes for input text, rest for solutions text
	Arena* solutions_arena;
	// Per-frame scratch
	Arena* frame_arena;

	// clay
	Font fonts[2];

	// ui state
	UI::TextEditState input_box;
	U64 input_box_limit;
	F64 blink_timer;			// cumulative blink timer (oscillates from 0-1) that resets on input events
	String8 placeholder_text; 	// "Enter equation";
	B32 need_placeholder;
	ScrollBarData scrollbar_data;

	expr::Node* root_node;

	std::vector<NodeBox> node_boxes;	// TODO(sb): replace vector when done prototyping
	// std::vector<NodeBox> highlight_boxes;
	std::unordered_map<U64, U32> node_boxes_cache;	// node id -> node box vector index

	// TODO(me): cross-platform solution for paths, in windows msvc can embed into exe, in linux and mac can with #embed or can package into .App or .AppInstaller
	// paths
	// String8 data_directory_path
	// String8 font_path;

	// Events
	OS::EventList* events; // go on frame arena because raylib produces key events every frame
	// App config struct? See from cactus image viewer and this emacs-like editor repo you starred from clay discords
};

State* app_state = nullptr;

// Clay stuff
const int FONT_ID_BODY_16 = 0;
Clay_Color COLOR_WHITE = { 255, 255, 255, 255 }; 

void HandleClayErrors(Clay_ErrorData errorData) {
    // See the Clay_ErrorData struct for more information
    printf("%s", errorData.errorText.chars);
    // switch (errorData.errorType)
    // {
    // 	case CLAY_ERROR_TYPE_TEXT_MEASUREMENT_CAPACITY_EXCEEDED:
	// 	{
	// 		ArenaClear(app_state->clay_arena);
	// 		S32 clay_max_text_cache = Clay_GetMaxMeasureTextCacheWordCount();
	// 		Clay_SetMaxMeasureTextCacheWordCount(clay_max_text_cache*2);
	// 		U64 min_memory_size = Clay_MinMemorySize();
	// 		void* clay_mem = PushArrayNoZero(app_state->clay_arena, U8, min_memory_size*2);

	// 		Clay_Arena clay_memory = Clay_CreateArenaWithCapacityAndMemory(min_memory_size, clay_mem);
	// 		Clay_Initialize(clay_memory, Clay_Dimensions{(float)GetScreenWidth(), (float)GetScreenHeight()}, Clay_ErrorHandler{HandleClayErrors});
	// 		app_state->fonts[FONT_ID_BODY_16] = LoadFontEx("D:\\Coding\\dsp-project-refactor\\data\\Roboto-Regular.ttf", 48, 0, 400); 
	// 	    SetTextureFilter(app_state->fonts[FONT_ID_BODY_16].texture, TEXTURE_FILTER_BILINEAR);
	// 	    Clay_SetMeasureTextFunction(Raylib_MeasureText, app_state->fonts);
	// 	}
    // }
    // switch(errorData.errorType) {
    //     // etc
    // }
}

internal void Initialise(Arena* arena);
internal void BuildUI();
internal void Shutdown();

inline Clay_String ClayStringFromString8(const String8& s)
{
	return Clay_String(
		false, 
		(S32)s.size,
		reinterpret_cast<const char*>(s.str)
		);
}



}	// namespace App