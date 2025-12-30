/*  date = December 09th 2025 05:50 PM */


//////////////////////////////
//- Build Options

#define BUILD_TITLE "main"
#define BUILD_ENTRY_POINT_DEFINING_UNIT 1 // export to batch file
#define BUILD_COMMAND_LINE_INTERFACE 1 // export to batch file

//////////////////////////////
//- Includes

//- [h] root
#include "base/base_inc.h"
#include "os/os_inc.h"
#include "parse/parse_inc.h"
#include "ui/ui_inc.h"

//- foreign includes
#define CLAY_IMPLEMENTATION
#include "third_party/clay/clay.h"
#include "third_party/clay/clay_renderer_raylib.h"
#include "tester/simpletest.h"
#include "tester/simpletest.cpp"

//- [h] app
#include "entry/main_core.h"

//- [cpp] app
#include "entry/main_core.cpp"

//- [cpp] root
#include "base/base_inc.cpp"
#include "os/os_inc.cpp"
#include "parse/parse_inc.cpp"
#include "ui/ui_inc.cpp"


B32 TextOpHasEffect(UI::TextOp* op, UI::TextEditState* before);
B32 TextOpHasEffect(UI::TextOp* op, UI::TextEditState* before)
{
    B32 result = 0;
    if (op->range.min != op->range.max ||   // non-empty replaced range (delete or overwrite)
        op->replace_string.size != 0 ||     // inserted text
        op->new_cursor != before->cursor || // cursor moved
        op->new_mark != before->mark)       // selection / mark changed
    {
        result = 1;
    }

    return result;
}

internal void 
EntryPoint(U64 argument_count, char** arguments) 
{

    ArenaTemp scratch = ScratchBegin(nullptr, 0);   // PROGRAM STATE lives on scratch
        
	App::Initialise(scratch.arena);

    // TODO(sb): autoscroll
    // S64 old_autoscroll_bounds = 0;
    // S64 autoscroll_bounds = 0;
    // B32 did_autoscroll = false;
    B32 debug_enabled = false;

	while (!WindowShouldClose()) 
	{
// -------------------------------------------------------------------------------------------------
		ArenaClear(App::app_state->frame_arena);
        F32 dt = GetFrameTime();
        // did_autoscroll = false;

        App::app_state->events = OS::GetKeyboardEvents(App::app_state->frame_arena);
        OS::Event* first_event = App::app_state->events->first;

        for EachNode(event, OS::Event, first_event)
        {

            // TODO(sb): if (focused) wrap all of text edit code in this

            // update text edit widget from keyboard events
            UI::TextAction action = UI::TextActionFromEvent(event);
            
            UI::TextOp op = UI::TextOpFromStateAndAction(
                App::app_state->frame_arena,
                App::app_state->input_box.text,
                &App::app_state->input_box,
                &action
            );
            
            B32 taken = TextOpHasEffect(&op, &App::app_state->input_box);
            if (taken) 
            {

                UI::ApplyTextOp(
                    App::app_state->string_arena,
                    &App::app_state->input_box,
                    &op
                );
                

                OS::EatEvent(App::app_state->events, event);
            }

            // clay debug
            if ( IsKeyDown(KEY_LEFT_CONTROL))
            {
                if (IsKeyPressed(KEY_D))
                {

                    debug_enabled = !debug_enabled;
                    Clay_SetDebugModeEnabled(debug_enabled);
                }
            }
            
            // TODO(sb): auto scroll is sloppy

            // Clay_ElementId id = Clay_GetElementId(CLAY_STRING("INPUT_BOX"));
            // Clay_ElementData data = Clay_GetElementData(id);
            // Clay_BoundingBox bbox = data.boundingBox;
            // autoscroll_bounds = App::app_state->input_box.scroll_offset_x;

            // EnsureCursorVisible(&App::app_state->input_box, App::app_state->fonts[0], bbox.width);

            // // either autoscroll via cursor, or scroll with
            // if (autoscroll_bounds != old_autoscroll_bounds)
            // {
            //     Clay_UpdateScrollContainers(true, Clay_Vector2{-(App::app_state->input_box.scroll_offset_x), 0}, dt);
            //     old_autoscroll_bounds = autoscroll_bounds;
            //     did_autoscroll = true;
            // }
            // debug
            // char debug_buffer[64];
            // sb_stbsp_snprintf(debug_buffer, sizeof(debug_buffer), "%s%f", "Scroll offset: ", App::app_state->input_box.scroll_offset_x);
            // OutputDebugStringA(debug_buffer);
            // OutputDebugStringA("\n");
            


        }

		Clay_SetLayoutDimensions(Clay_Dimensions {(float)GetScreenWidth(), (float)GetScreenHeight() });


// -------------------------------------------------------------------------------------------------
		
        Vector2 mousePosition = GetMousePosition();
        Vector2 scrollDelta = GetMouseWheelMoveV();

        F32 scroll_multiplier = 10.0f;
        scrollDelta.x *= scroll_multiplier;
        scrollDelta.y *= scroll_multiplier;

        Clay_SetPointerState(Clay_Vector2{ mousePosition.x, mousePosition.y }, IsMouseButtonDown(0) );
        // if (!did_autoscroll)
            Clay_UpdateScrollContainers(true, Clay_Vector2{ scrollDelta.x, scrollDelta.y }, dt );
		
       
// -------------------------------------------------------------------------------------------------


	    // Compute layout
	    Clay_BeginLayout();
	    App::BuildUI();
		Clay_RenderCommandArray renderCommands = Clay_EndLayout();

		// Render layout
		BeginDrawing();
		ClearBackground(BLACK);
		Clay_Raylib_Render(renderCommands, App::app_state->fonts);
        bool text_input_is_focused = true; // debug, make it work on mouse pointer clicks
        // render cursor
        if (text_input_is_focused)
        {
            RenderTextCursor(&App::app_state->input_box, App::app_state->fonts);
        }
		EndDrawing();
	}
	
    ScratchEnd(scratch);
	App::Shutdown();
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