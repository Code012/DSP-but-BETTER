/*  date = December 09th 2025 06:15 PM */ 

internal void RenderTextCursor(UI::TextEditState* state, Font* fonts);
internal void EnsureCursorVisible(UI::TextEditState* state, Font font, float box_width);

#define INPUT_TEXT_OFFSET 512

namespace App
{


struct State
{	
	// Permanent arenas
	Arena* clay_arena;
	Arena* string_arena;		// 512 bytes for input text (have to restrict because of some clay stuff i dont understand, check todo.txt for reason), rest for solutions text
	// Per-frame scratch
	Arena* frame_arena;

	// clay
	Font fonts[1];

	// ui state
	UI::TextEditState input_box;
	U64 input_box_limit;
	F64 blink_timer;		// cumulative blink timer (oscillates from 0-1) that resets on input events
	// String8 input_box_default // "enter equation";

	// TODO(me): cross-platform solution for paths, in windows msvc can embed into exe, in linux and mac can with #embed or can package into .App or .AppInstaller
	// paths
	// String8 data_directory_path
	// String8 font_path;

	// Events
	OS::EventList* events; // go on frame arena because raylib produces key events every frame
	// App config struct?
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
		true, 
		(S32)s.size,
		reinterpret_cast<const char*>(s.str)
		);
}
}	// namespace App