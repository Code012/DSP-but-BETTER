/*  date = December 09th 2025 06:15 PM */

internal void App_Initialise()
{
	// initialise app state 
	Arena* arena = ArenaAlloc();
	app_state = PushArray(arena, AppState, 1);
	app_state->arena = arena;
	app_state->frame_arena = ArenaAlloc();

	// initialise clay and raylib
	Clay_Raylib_Initialize(1024, 768, "Introducing Clay Demo", FLAG_WINDOW_RESIZABLE | FLAG_MSAA_4X_HINT | FLAG_VSYNC_HINT); // Extra parameters to this function are new since the video was published
    SetTargetFPS(60);

	U64 min_memory_size = Clay_MinMemorySize();
	void* clay_mem = PushArrayNoZero(arena, U8, min_memory_size);

	Clay_Arena clay_memory = Clay_CreateArenaWithCapacityAndMemory(min_memory_size, clay_mem);
	Clay_Initialize(clay_memory, Clay_Dimensions{(float)GetScreenWidth(), (float)GetScreenHeight()}, Clay_ErrorHandler{HandleClayErrors});
	// TODO(me): embed textures, look at discord martins
    app_state->fonts[FONT_ID_BODY_16] = LoadFontEx("../data/Roboto-Regular.ttf", 48, 0, 400);
    SetTextureFilter(app_state->fonts[FONT_ID_BODY_16].texture, TEXTURE_FILTER_BILINEAR);
    Clay_SetMeasureTextFunction(Raylib_MeasureText, app_state->fonts);

    // debug tools
    Clay_SetDebugModeEnabled(true);
}

internal void App_BuildUI()
{
	// TODO(me): Look at clay floating elements in docs for modals (settings window)
	// colours
	Clay_Color root_background = {254, 247, 245, 255}; 

	// specific configs
	Clay_Color input_box_submit_button_colour = { 255, 167, 109, 255 };

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
				},
				.backgroundColor = COLOR_WHITE,
				.cornerRadius = {8, 0, 8, 0},
				.border = {
					.color = {217, 218, 223, 255},	// greyish
					.width = {1, 1, 1, 1},
				}
			}) {};
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

internal void App_Shutdown()
{
	Clay_Raylib_Close();
}