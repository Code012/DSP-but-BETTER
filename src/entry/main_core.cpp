/*  date = December 09th 2025 06:15 PM */


// internal void 
// EnsureCursorVisible(UI::TextEditState* state, Font font, float box_width)
// {
//     // Measure text up to cursor
//     char temp_buffer[1024];
//     S64 cursor_pos = ClampTop(state->cursor, state->text.size);
    
//     memcpy(temp_buffer, state->text.str, cursor_pos);
//     temp_buffer[cursor_pos] = '\0';
    
//     Vector2 cursor_text_size = MeasureTextEx(font, temp_buffer, 24.0f, 0.0f);
//     float cursor_x_absolute = cursor_text_size.x; // Position in the full text
    
//     // Calculate cursor position relative to the visible area
//     float cursor_x_relative = cursor_x_absolute - state->scroll_offset_x;
    
    
//     // Cursor went off the right edge - scroll right
//     if (cursor_x_relative > box_width )
//     {
//         state->scroll_offset_x = cursor_x_absolute - (box_width + 10) ;
//     }
    
//     // Cursor went off the left edge - scroll left
//     if (cursor_x_relative < 0)
//     {
//         state->scroll_offset_x = cursor_x_absolute ;
//     }
    
//     // Don't scroll past the beginning
//     if (state->scroll_offset_x < 0)
//     {
//         state->scroll_offset_x = 0;
//     }
// }

internal void 
RenderTextCursor(UI::TextEditState* state, Font* fonts)
{
	Clay_ElementId id = Clay_GetElementId(CLAY_STRING("INPUT_BOX"));
	Clay_ScrollContainerData scroll_data = Clay_GetScrollContainerData(id);
	Clay_ElementData data = Clay_GetElementData(id);
	Clay_BoundingBox bbox = data.boundingBox;
	Font font_to_use = fonts[0];

	// debug
	char debug_buffer[64];
	sb_stbsp_snprintf(debug_buffer, sizeof(debug_buffer), "%s%f", "Scroll offset: ", scroll_data.scrollPosition->x);
    DrawTextEx(font_to_use, debug_buffer, Vector2{0, 0}, 16.0f, 0.0f, Color{0, 0, 0, 255}); 


	BeginScissorMode((int)bbox.x, (int)bbox.y, (int)bbox.width+2, (int)bbox.height);

	char temp_buffer[1024];
	S64 cursor_pos = (S64)state->cursor;
	// if (cursor_pos > state->text.size)
	// 	cursor_pos = (S64)state->text.size;
	ClampTop(cursor_pos, (S64)state->text.size);

	memcpy(temp_buffer, state->text.str, cursor_pos);
	temp_buffer[cursor_pos] = '\0';

	Vector2 text_size = MeasureTextEx(font_to_use, temp_buffer, 24.0f, 0.0f);	//TODO(sb): pass the font size and letter spacing as a global variable so its the same

	local_persist F64 blink_timer = 0.0;
	blink_timer += GetFrameTime();

	if (fmod(blink_timer, 1.0) < 0.5)
	{
		F32 cursor_x = bbox.x + text_size.x + scroll_data.scrollPosition->x;
		DrawRectangle((int)(cursor_x), (int)bbox.y, 2, (int)bbox.height, {0, 0, 0, 255});
	}

	EndScissorMode();
}

namespace App
{

internal void Initialise(Arena* arena)
{
	// initialise app state 
	app_state = PushArray(arena, State, 1);
	app_state->clay_arena = ArenaAlloc();
	app_state->string_arena = ArenaAlloc(KiB(64), true);
	app_state->frame_arena = ArenaAlloc();

	// initialise clay and raylib
	Clay_Raylib_Initialize(1024, 768, "Introducing Clay Demo", FLAG_WINDOW_RESIZABLE | FLAG_MSAA_4X_HINT | FLAG_VSYNC_HINT); // Extra parameters to this function are new since the video was published
    SetTargetFPS(60);
	Clay_SetMaxMeasureTextCacheWordCount(KiB(64));

	U64 min_memory_size = Clay_MinMemorySize();
	void* clay_mem = PushArrayNoZero(app_state->clay_arena, U8, min_memory_size);

	Clay_Arena clay_memory = Clay_CreateArenaWithCapacityAndMemory(min_memory_size, clay_mem);
	

	Clay_Initialize(clay_memory, Clay_Dimensions{(float)GetScreenWidth(), (float)GetScreenHeight()}, Clay_ErrorHandler{HandleClayErrors});
	// TODO(me): embed textures, look at discord martins
	//NOTE(sb): giving absoloute path so that radbg can find it
    app_state->fonts[FONT_ID_BODY_16] = LoadFontEx("D:\\Coding\\dsp-project-refactor\\data\\Roboto-Regular.ttf", 48, 0, 400); 
    SetTextureFilter(app_state->fonts[FONT_ID_BODY_16].texture, TEXTURE_FILTER_BILINEAR);
    Clay_SetMeasureTextFunction(Raylib_MeasureText, app_state->fonts);

    // debug tools
    Clay_SetDebugModeEnabled(true);

    // text widget
    app_state->input_box = zero_struct;
    app_state->input_box.text.str = PushArrayNoZero(app_state->string_arena, U8, INPUT_TEXT_OFFSET);
    app_state->input_box.cursor = 0;
    app_state->input_box.mark = 0;
    app_state->input_box.scroll_offset_x = 0.0f;
}

internal void BuildUI()
{
	// TODO(me): Look at clay floating elements in docs for modals (settings window)
	// colours
	Clay_Color root_background = {254, 247, 245, 255}; 

	// specific configs
	Clay_Color input_box_submit_button_colour = { 255, 167, 109, 255 };

	#if 0
	// text edit widget 
	CustomLayoutElement* input_element = PushArray(app_state->frame_arena, CustomLayoutElement, 1);
	input_element->type = CUSTOM_LAYOUT_ELEMENT_TYPE_TEXT_EDIT;
	input_element->customData.text_edit.state = &app_state->input_box;
    input_element->customData.text_edit.text_colour = BLACK;
    input_element->customData.text_edit.selection_colour = ColorAlpha(BLUE, 0.3f);
    input_element->customData.text_edit.cursor_colour = RED;
    input_element->customData.text_edit.font_size = 20;
	#endif

	// common configs
	Clay_Sizing layout_expand = { .width = CLAY_SIZING_GROW(), .height = CLAY_SIZING_GROW() };

// -------------------------------------------------------------------------------------------------
	//- ROOT
	CLAY(CLAY_ID("ROOT"), {
		.layout = {
			.sizing = layout_expand,
			.padding = { 16, 16, 0, 16 },
			.childGap = 16,
			.layoutDirection = CLAY_TOP_TO_BOTTOM,
		},
		.backgroundColor = root_background,
	}) 
	//- Root Children 
	{
// -------------------------------------------------------------------------------------------------
		//- INNER_ROOT_TOP
		CLAY(CLAY_ID("INNER_ROOT_TOP"), {
			.layout = {
				.sizing = layout_expand,
				.padding = CLAY_PADDING_ALL(16),
				.layoutDirection = CLAY_TOP_TO_BOTTOM,
			}
		}) 
		//- INNER_ROOT_TOP Children
		{

		};
		//- INNER_ROOT_TOP END
// -------------------------------------------------------------------------------------------------
		//- INNER_ROOT_BOT
		CLAY(CLAY_ID("INNER_ROOT_BOT"), {
			.layout = {
				.sizing = {
					.width = CLAY_SIZING_GROW(),
					.height = CLAY_SIZING_GROW(80, 80) // never smaller or larger than 80
				},
				.padding = {0, 0, 16, 16},
				.childGap = 3,
				.layoutDirection = CLAY_LEFT_TO_RIGHT,
			}
		}) 
		//- INNER_ROOT_BOT Children
		{
// -------------------------------------------------------------------------------------------------
			//- INPUT_BOX 
			CLAY(CLAY_ID("INPUT_BOX"), {
				.layout = {
					.sizing = layout_expand,
					.childAlignment = {
						.x = CLAY_ALIGN_X_LEFT,
						.y = CLAY_ALIGN_Y_CENTER,
					},
				},
				.backgroundColor = COLOR_WHITE,
				.cornerRadius = {8, 0, 8, 0},
				.clip = {	
					.horizontal = true,
					.childOffset = Clay_GetScrollOffset(), // scroll doesnt work
				},
				.border = {
					.color = {217, 218, 223, 255},	// greyish
					.width = {1, 1, 1, 1},
				}
			}) 
			//- INPUT_BOX Children
			{
// -------------------------------------------------------------------------------------------------
				CLAY_TEXT(ClayStringFromString8(app_state->input_box.text), CLAY_TEXT_CONFIG({
                    .textColor = {0, 0, 0, 255},
                    .fontId = FONT_ID_BODY_16,
                    .fontSize = 24,
	                }));
// -------------------------------------------------------------------------------------------------
			};
			
			
			//- INPUT_BOX End
// -------------------------------------------------------------------------------------------------
			//- INPUT_BOX_SUBMIT_BUTTON
			CLAY(CLAY_ID("INPUT_BOX_SUBMIT_BUTTON"), {
				.layout = {
					.sizing = {
						.width = CLAY_SIZING_FIT(),
						.height = CLAY_SIZING_GROW(),
					},
					.padding = CLAY_PADDING_ALL(16),
					.childAlignment = {
						.x = CLAY_ALIGN_X_CENTER,
						.y = CLAY_ALIGN_Y_CENTER,
					},
				},
				.backgroundColor = input_box_submit_button_colour,
				// .cornerRadius = {0, 8, 0, 8}, buggy doesnt work
				.border = {
					.color = {255, 131, 71, 255},
					.width = {1, 1, 1, 1}
				}
			}) 
			//- INPUT_BOX_SUBMITE_BUTTON Children
			{
				CLAY_TEXT(CLAY_STRING("Solve"), CLAY_TEXT_CONFIG({
					.textColor = COLOR_WHITE,
					.fontId = FONT_ID_BODY_16,
					.fontSize = 16,
				}));
			};
			//- INPUT_BOX_SUBMIT_BUTTON End
// -------------------------------------------------------------------------------------------------
		};
		//- INNER_ROOT END
// -------------------------------------------------------------------------------------------------
	};
	//- Root End
// -------------------------------------------------------------------------------------------------

}

internal void Shutdown()
{
	Clay_Raylib_Close();
}

} // namespace App