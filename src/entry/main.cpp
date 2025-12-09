/*  date = December 09th 2025 05:50 PM */


//////////////////////////////
//- Build Options

#define BUILD_TITLE "main"
#define BUILD_ENTRY_POINT_DEFINING_UNIT 1 // export to batch file
#define BUILD_COMMAND_LINE_INTERFACE 1 // export to batch file

//////////////////////////////
//- Includes

//- foreign includes
#define CLAY_IMPLEMENTATION
#include "third_party/clay/clay.h"
#include "third_party/clay/clay_renderer_raylib.h"

//- [h] root
#include "base/base_inc.h"
#include "os/os_inc.h"
#include "parse/parse_inc.h"
#include "tester/simpletest.h"

//- [h] app
#include "entry/main_core.h"

//- [cpp] app
#include "entry/main_core.cpp"

//- [cpp] root
#include "base/base_inc.cpp"
#include "os/os_inc.cpp"
#include "parse/parse_inc.cpp"
#include "tester/simpletest.cpp"






internal void 
EntryPoint(U64 argument_count, char** arguments) 
{
	// Arena* clay_ui_arena = ArenaAlloc();
	// void* clay_ui_mem = (void *)(&clay_ui_arena + clay_ui_arena->pos);

    
	App_Initialise();

	while (!WindowShouldClose()) 
	{
// -------------------------------------------------------------------------------------------------
		Clay_SetLayoutDimensions(Clay_Dimensions {(float)GetScreenWidth(), (float)GetScreenHeight() });

		ArenaClear(app_state->frame_arena);

// -------------------------------------------------------------------------------------------------
		F32 dt = GetFrameTime();

		Vector2 mousePosition = GetMousePosition();
		Vector2 scrollDelta = GetMouseWheelMoveV();

		F32 scroll_multiplier = 10.0f;
		scrollDelta.x *= scroll_multiplier;
		scrollDelta.y *= scroll_multiplier;

		Clay_SetPointerState(Clay_Vector2{ mousePosition.x, mousePosition.y }, IsMouseButtonDown(0) );
		Clay_UpdateScrollContainers(true, Clay_Vector2{ scrollDelta.x, scrollDelta.y }, dt );
// -------------------------------------------------------------------------------------------------


	    // Compute layout
	    Clay_BeginLayout();
	    App_BuildUI();
		Clay_RenderCommandArray renderCommands = Clay_EndLayout();

		// Render layout
		BeginDrawing();
		ClearBackground(BLACK);
		Clay_Raylib_Render(renderCommands, app_state->fonts);
		EndDrawing();
	}
	
	App_Shutdown();
}

// internal void
// EntryPoint(U64 argument_count, char** arguments)
// {
// 	(void) argument_count;
// 	(void) arguments;

// 	App_Initialise();

// 	while (!WindowShouldClose())
// 	{
// 		ArenaClear(app->state->frame_arena);

// 		F32 dt = GetFrameTime();

// 		App_ProcessInput(dt);
// 		App_BuildUI();
// 		App_Render();
// 	}

// 	App_Shutdown();
// }

#if 0
typedef struct Clay_ElementDeclaration {
    // Controls various settings that affect the size and position of an element, as well as the sizes and positions of any child elements.
    Clay_LayoutConfig layout;
    // Controls the background color of the resulting element.
    // By convention specified as 0-255, but interpretation is up to the renderer.
    // If no other config is specified, .backgroundColor will generate a RECTANGLE render command, otherwise it will be passed as a property to IMAGE or CUSTOM render commands.
    Clay_Color backgroundColor;
    // Controls the "radius", or corner rounding of elements, including rectangles, borders and images.
    Clay_CornerRadius cornerRadius;
    // Controls settings related to aspect ratio scaling.
    Clay_AspectRatioElementConfig aspectRatio;
    // Controls settings related to image elements.
    Clay_ImageElementConfig image;
    // Controls whether and how an element "floats", which means it layers over the top of other elements in z order, and doesn't affect the position and size of siblings or parent elements.
    // Note: in order to activate floating, .floating.attachTo must be set to something other than the default value.
    Clay_FloatingElementConfig floating;
    // Used to create CUSTOM render commands, usually to render element types not supported by Clay.
    Clay_CustomElementConfig custom;
    // Controls whether an element should clip its contents, as well as providing child x,y offset configuration for scrolling.
    Clay_ClipElementConfig clip;
    // Controls settings related to element borders, and will generate BORDER render commands.
    Clay_BorderElementConfig border;
    // A pointer that will be transparently passed through to resulting render commands.
    void *userData;
} Clay_ElementDeclaration;

typedef struct Clay_LayoutConfig {
    Clay_Sizing sizing; // Controls the sizing of this element inside it's parent container, including FIT, GROW, PERCENT and FIXED sizing.
    Clay_Padding padding; // Controls "padding" in pixels, which is a gap between the bounding box of this element and where its children will be placed.
    uint16_t childGap; // Controls the gap in pixels between child elements along the layout axis (horizontal gap for LEFT_TO_RIGHT, vertical gap for TOP_TO_BOTTOM).
    Clay_ChildAlignment childAlignment; // Controls how child elements are aligned on each axis.
    Clay_LayoutDirection layoutDirection; // Controls the direction in which child elements will be automatically laid out.
} Clay_LayoutConfig;

// Controls various functionality related to text elements.
typedef struct Clay_TextElementConfig {
    // A pointer that will be transparently passed through to the resulting render command.
    void *userData;
    // The RGBA color of the font to render, conventionally specified as 0-255.
    Clay_Color textColor;
    // An integer transparently passed to Clay_MeasureText to identify the font to use.
    // The debug view will pass fontId = 0 for its internal text.
    uint16_t fontId;
    // Controls the size of the font. Handled by the function provided to Clay_MeasureText.
    uint16_t fontSize;
    // Controls extra horizontal spacing between characters. Handled by the function provided to Clay_MeasureText.
    uint16_t letterSpacing;
    // Controls additional vertical space between wrapped lines of text.
    uint16_t lineHeight;
    // Controls how text "wraps", that is how it is broken into multiple lines when there is insufficient horizontal space.
    // CLAY_TEXT_WRAP_WORDS (default) breaks on whitespace characters.
    // CLAY_TEXT_WRAP_NEWLINES doesn't break on space characters, only on newlines.
    // CLAY_TEXT_WRAP_NONE disables wrapping entirely.
    Clay_TextElementConfigWrapMode wrapMode;
    // Controls how wrapped lines of text are horizontally aligned within the outer text bounding box.
    // CLAY_TEXT_ALIGN_LEFT (default) - Horizontally aligns wrapped lines of text to the left hand side of their bounding box.
    // CLAY_TEXT_ALIGN_CENTER - Horizontally aligns wrapped lines of text to the center of their bounding box.
    // CLAY_TEXT_ALIGN_RIGHT - Horizontally aligns wrapped lines of text to the right hand side of their bounding box.
    Clay_TextAlignment textAlignment;
} Clay_TextElementConfig;
#endif