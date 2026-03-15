/*  date = December 09th 2025 05:50 PM */


//////////////////////////////
//- Build Options

#define BUILD_TITLE "main"
#define BUILD_ENTRY_POINT_DEFINING_UNIT 1 // export to batch file
#define BUILD_COMMAND_LINE_INTERFACE 1 // export to batch file

//////////////////////////////
//- Includes

//- stl
#include <iostream>
#include <unordered_map>  
#include <vector>   // TODO(sb): GET RID OF THIS WHEN DONE PROTOTYPING

//- [h] root
#include "base/base_inc.h"
#include "os/os_inc.h"
#include "expr/expr_parse.h"
// #include "parse/parse_inc.h"
// #include "algebra/algebra_inc.h"
#include "ui/ui_inc.h"

//- foreign includes
#define CLAY_IMPLEMENTATION
#include "third_party/clay/clay.h"
#include "third_party/raylib/include/raylib.h"
#include "third_party/raylib/include/raymath.h"
#include "third_party/clay/clay_renderer_raylib.h" // when rewriting this move this to the renderer layer, move to its own render folder

// #include "tester/simpletest.h"
// #include "tester/simpletest.cpp"

//- [h] app
#include "entry/main_core.h"

//- [cpp] app
#include "entry/main_core.cpp"

//- [cpp] root
#include "base/base_inc.cpp"
#include "os/os_inc.cpp"
#include "expr/expr_parse.cpp"
// #include "parse/parse_inc.cpp"
// #include "algebra/algebra_inc.cpp"
#include "ui/ui_inc.cpp"







B32 TextOpHasEffect(UI::TextOp* op, UI::TextEditState* before);
B32 TextOpHasEffect(UI::TextOp* op, UI::TextEditState* before)
{
    B32 result = 0;
    if (op->range.min() != op->range.max() ||   // non-empty replaced range (delete or overwrite)
        op->replace_string.size != 0 ||     // inserted text
        op->new_cursor != before->cursor || // cursor moved
        op->new_mark != before->mark)       // selection / mark changed
    {
        result = 1;
    }

    return result;
}

internal Rng2F32 RectUnion(Rng2F32 a, Rng2F32 b)   // min of mins, max of maxes. easy way to think about it
{
    if (a.x0 > b.x0) a.x0 = b.x0;
    if (a.x1 < b.x1) a.x1 = b.x1;
    if (a.y0 > b.y0) a.y0 = b.y0;
    if (a.y1 < b.y1) a.y1 = b.y1;

    return a;
}


// Performs RectUnion on operator bounding boxes anad its children for highlight_rect
// internal Rng2F32 ComputeOperatorBounds(expr::Node* node)
// {
//     using namespace expr;
//     using namespace App;

//     NodeBox& node_box = FindBoxFromNode(node);
//     node_box.highlight_rect = node_box.rect;

//     switch (node->kind)
//     {
//         case NodeKind::Number: {
//             return node_box.highlight_rect;
//         } break;

//         case NodeKind::BinaryOp: {
//             Rng2F32 left_rect  = ComputeOperatorBounds(node->bin_left);
//             Rng2F32 right_rect = ComputeOperatorBounds(node->bin_right);
//             node_box.highlight_rect = RectUnion(node_box.highlight_rect, left_rect);        // NOTE(sb): would this be nicer as node_rect |= left_rect?
//             node_box.highlight_rect = RectUnion(node_box.highlight_rect, right_rect);
//         } break;

//         case NodeKind::NaryOp: {
//             Rng2F32 rect{std::numeric_limits<F32>::max(), std::numeric_limits<F32>::max(), std::numeric_limits<F32>::min(), std::numeric_limits<F32>::min()};
//             for (Node* it = node->nary_first; !NodeIsNil(it); it = it->nary_next)
//             {
//                 Rng2F32 child_rect = ComputeOperatorBounds(it);
//                 rect = RectUnion(rect, child_rect);
//             }

//             node_box.highlight_rect = RectUnion(node_box.highlight_rect, rect);

//         } break;

//         default:
//             Assert(false && "unexpected node type");
//     }
    
//     return node_box.highlight_rect;
// }

internal Rng2F32 RectExpand(Rng2F32 a, Rng2F32 b)
{
    if (a.x0 > b.x1) a.x0 = b.x1;
    if (a.y0 > b.y1) a.y0 = b.y1;
    if (a.x1 < b.x0) a.x1 = b.x0;
    if (a.y1 < b.y0) a.y1 = b.y0;

    return a;
}

internal void MarkSubTree(expr::Node* node)
{   
    using namespace expr;
    node->visit_mark = App::app_state->current_mark;

    switch(node->kind)
    {
         case NodeKind::BinaryOp:
            MarkSubTree(node->bin_left);
            MarkSubTree(node->bin_right);
            break;

        case NodeKind::NaryOp:
            for (Node* it = node->nary_first; !NodeIsNil(it); it = it->nary_next)
                MarkSubTree(it);
            break;

        case NodeKind::Number:
            break;
    }

}

// this only wokrs because the way the nodeboxes are laid out in the vector are in in-order which is the exact order they appear visually left-to-right on screen.
// so satisfying this just works
// internal void ComputeHitRects(expr::Node* node)
// {
//     using namespace App;

//     // these get ignored in loop so i do it here
//     auto& first = app_state->node_boxes.front();
//     auto& last = app_state->node_boxes.back();
//     first.hit_rect = first.rect;
//     last.hit_rect = last.rect;

//     for (auto it = app_state->node_boxes.begin() + 1; // ignoring first and last (left and right on screen) 
//         it != app_state->node_boxes.end() - 1; 
//         ++it)
//     {
//         auto& prev = *(it - 1);
//         auto& curr = *it;
//         auto& next = *(it + 1);

//         curr.hit_rect = curr.rect;

//         curr.hit_rect = RectExpand(curr.hit_rect, prev.hit_rect);
//         curr.hit_rect = RectExpand(curr.hit_rect, next.hit_rect);
//     }
// }

internal void 
EntryPoint(U64 argument_count, char** arguments) 
{

    ArenaTemp scratch = ScratchBegin(nullptr, 0);   // PROGRAM STATE lives on scratch
        
	App::Initialise(scratch.arena);
    App::app_state->node_boxes.reserve(256); 

    // TODO(sb): autoscroll
    // S64 old_autoscroll_bounds = 0;
    // S64 autoscroll_bounds = 0;
    // B32 did_autoscroll = false;
    B32 debug_enabled = true;
    B32 submit = false;

	while (!WindowShouldClose()) 
	{
        // TODO(sb): have some "reinitialise_clay" logic if an error is caught in HandleClayErrors, look at clay_examples/main.c for how to do it

		ArenaClear(App::app_state->frame_arena);
        App::app_state->highlight_root = nullptr;
        App::app_state->hovered_box = nullptr;
        F32 dt = GetFrameTime();
        submit = false;
// -------------------------------------------------------------------------------------------------
        App::app_state->events = OS::GetKeyboardEvents(App::app_state->frame_arena);
        OS::Event* first_event = App::app_state->events->first;

        for EachNode(event, OS::Event, first_event)
        {
            App::app_state->blink_timer = 0.0;  // reset blink timer on input event because blink_timer is cumulative.

            if (event->kind == OS::EventKind::Press)
            {
                if (event->key == OS::Key::Enter)
                {
                    submit = true;
                    break;              // don't process any more events
                }
            }

            // TODO(sb): if (focused) wrap all of text edit code in this

            // Update text edit widget from keyboard events
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
                Clay_ResetMeasureTextCache();       // without this, writing words (characters delimited by space) crashes the program at a certain length

                UI::ApplyTextOp(
                    // App::app_state->string_arena,
                    &App::app_state->input_box,
                    &op
                );
                
                OS::EatEvent(App::app_state->events, event);
            }
        }
// -------------------------------------------------------------------------------------------------

        // Symbolic Algebra Calc
        // TODO(sb): Handle submit button pointer click too, its just enter for now
        if (submit)
        {
            expr::Parser parser{App::app_state->solutions_arena, App::app_state->input_box.text};   
            expr::ParseResult parse_result = expr::ParseFromText(&parser, App::app_state->input_box.text);
            expr::DebugPrintParseResult(parse_result, App::app_state->input_box.text);
        
            if (parse_result.msgs.count > 0)
            {
                // TODO(sb): Display errors and warnings in ui
            }
            ArenaClear(App::app_state->solutions_arena);
            
            
            // Print using root node
            // 2 / (32 + 43 + 50 + (-10))
            /*
                        /
                       / \  
                      2  __+_____
                        |  |  |  | 
                        32 43 50 -
                                 |
                                 10

                      or more accurately

                        /
                       / \
                      2   + -> 32 -> 43 -> 50 -> -
                                                 |
                                                 10

                all nodes except for ones that represnt a meaningful visual grouping, like binary or nary
                copy the parent's highlight_root. + highlight gropu is itself and is copied to all children, 
                even grand children like 10
            */
            expr::Node* sub_node  = PushStruct(App::app_state->solutions_arena, expr::Node);
            expr::Node* num_node1 = PushStruct(App::app_state->solutions_arena, expr::Node);
            expr::Node* prod_node = PushStruct(App::app_state->solutions_arena, expr::Node);
            expr::Node* pnum_node1 = PushStruct(App::app_state->solutions_arena, expr::Node);
            expr::Node* pnum_node2 = PushStruct(App::app_state->solutions_arena, expr::Node);
            expr::Node* pnum_node3 = PushStruct(App::app_state->solutions_arena, expr::Node);
            expr::Node* punary_node = PushStruct(App::app_state->solutions_arena, expr::Node);
            expr::Node* punary_num1 = PushStruct(App::app_state->solutions_arena, expr::Node);


            sub_node->bin_left = num_node1;
            sub_node->bin_right = prod_node;
            sub_node->bin_ops = expr::BinOpKind::Minus;
            sub_node->kind = expr::NodeKind::BinaryOp;
            sub_node->highlight_root = sub_node;  
            sub_node->id = 0;

            num_node1->kind        = expr::NodeKind::Number;  
            num_node1->number      = 2.0;
            num_node1->highlight_root = sub_node;
            num_node1->id          = 1;

            prod_node->kind         = expr::NodeKind::NaryOp;
            prod_node->nary_ops     = expr::NaryOpKind::Multiply;
            prod_node->nary_first   = pnum_node1;
            prod_node->nary_next    = pnum_node2;
            prod_node->num_operands = 3;
            prod_node->highlight_root = prod_node;
            prod_node->id           = 2;


            pnum_node1->nary_first  = pnum_node1;
            pnum_node1->nary_next   = pnum_node2;
            pnum_node1->kind        = expr::NodeKind::Number;  
            pnum_node1->number      = 32.0;
            pnum_node1->highlight_root = prod_node;
            pnum_node1->id          = 3;

            pnum_node2->nary_first  = pnum_node1; 
            pnum_node2->nary_next   = pnum_node3;
            pnum_node2->kind        = expr::NodeKind::Number;
            pnum_node2->number      = 43.0;
            pnum_node2->highlight_root = prod_node;
            pnum_node2->id          = 4;

            pnum_node3->nary_first  = pnum_node1; 
            pnum_node3->nary_next   = &expr::nil_node;//punary_node;
            pnum_node3->kind        = expr::NodeKind::Number;
            pnum_node3->number      = 50.0;
            pnum_node3->highlight_root = prod_node;
            pnum_node3->id          = 5;

            // punary_node->nary_next = pnum_node1;
            // punary_node->nary_next = &expr::nil_node;
            // punary_node->kind = expr::NodeKind::UnaryOp;
            // punary_node->un_ops = expr::UnOpKind::Negate;
            // punary_node->highlight_root = prod_node->highlight_root;
            // punary_node->id = 6;

            // punary_node->kind = expr::NodeKind::Number;
            // punary_node->number = 10.0;
            // punary_num1->highlight_root = punary_node->highlight_root;



            App::app_state->root_node = sub_node;
            // expr::Result algebra_result = expr::SimplifyWithSteps(App::app_state->solutions_arena, parse_result.root);
            // algebra::PrintSteps(algebra_result.steps);
        }

// -------------------------------------------------------------------------------------------------

        App::app_state->need_placeholder = App::app_state->input_box.text.size > 0 ? 0 : 1; // check if text was inputted

        Vector2 mouse_wheel_delta = GetMouseWheelMoveV();
        F32 scroll_multiplier = 10.0f;
        mouse_wheel_delta.x *= scroll_multiplier;
        mouse_wheel_delta.y *= scroll_multiplier;

        // clay debug
        if ( IsKeyDown(KEY_LEFT_CONTROL))
        {
            if (IsKeyPressed(KEY_D))
            {
                debug_enabled = !debug_enabled;
                Clay_SetDebugModeEnabled(debug_enabled);
            }
        }

// -------------------------------------------------------------------------------------------------
		//- Handle Scroll Containers
        Vector2 mouse_position = GetMousePosition();
        Clay_SetPointerState(Clay_Vector2{ mouse_position.x, mouse_position.y }, IsMouseButtonDown(0) && !App::app_state->scrollbar_data.mouse_down);
        Clay_SetLayoutDimensions(Clay_Dimensions{(float)GetScreenWidth(), (float)GetScreenHeight() });
        // if (!IsMouseButtonDown(0))
        // {
        //     App::app_state->scrollbar_data.mouse_down = false;
        // }

        // if (IsMouseButtonDown(0) && !App::app_state->scrollbar_data.mouse_down && Clay_PointerOver(Clay_GetElementId(CLAY_STRING("SCROLLBAR"))))
        // {
        //     Clay_ScrollContainerData scroll_container_data = Clay_GetScrollContainerData(Clay_GetElementId(CLAY_STRING("INPUT_BOX")));
        //     App::app_state->scrollbar_data.click_origin = Clay_Vector2{mouse_position.x, mouse_position.y};
        //     App::app_state->scrollbar_data.position_origin = *scroll_container_data.scrollPosition;
        //     App::app_state->scrollbar_data.mouse_down = true;
        // }
        // else if (App::app_state->scrollbar_data.mouse_down)
        // {
        //     Clay_ScrollContainerData scroll_container_data = Clay_GetScrollContainerData(Clay_GetElementId(CLAY_STRING("INPUT_BOX")));
        //     if (scroll_container_data.contentDimensions.width > 0) {
        //         Clay_Vector2 ratio = Clay_Vector2{
        //             scroll_container_data.contentDimensions.width / scroll_container_data.scrollContainerDimensions.width,
        //             scroll_container_data.contentDimensions.height / scroll_container_data.scrollContainerDimensions.height,
        //         };
        //         if (scroll_container_data.config.vertical) {
        //             scroll_container_data.scrollPosition->y = App::app_state->scrollbar_data.position_origin.y + (App::app_state->scrollbar_data.click_origin.y - mouse_position.y) * ratio.y;
        //         }
        //         if (scroll_container_data.config.horizontal) {
        //             scroll_container_data.scrollPosition->x = App::app_state->scrollbar_data.position_origin.x + (App::app_state->scrollbar_data.click_origin.x - mouse_position.x) * ratio.x;
        //         }
        //     }
        // }

        Clay_UpdateScrollContainers(true, Clay_Vector2{ mouse_wheel_delta.x, mouse_wheel_delta.y }, GetFrameTime());
		
       
// -------------------------------------------------------------------------------------------------

        // 

// -------------------------------------------------------------------------------------------------


	    // Compute layout
	    Clay_BeginLayout();
	    App::BuildUI();
		Clay_RenderCommandArray renderCommands = Clay_EndLayout();

		// Render layout
		BeginDrawing();
		ClearBackground(BLACK);
		Clay_Raylib_Render(renderCommands, App::app_state->fonts);
        char debug_buffer[1024];
        sb_stbsp_snprintf(debug_buffer, sizeof(debug_buffer), "%s%d", "Num chars: ", App::app_state->input_box.text.size);
        DrawTextEx(App::app_state->fonts[0], debug_buffer, Vector2{500, 0}, 16.0f, 0.0f, Color{0, 0, 0, 255}); 

        bool text_input_is_focused = true; // debug, make it work on mouse pointer clicks
        // render cursor
        if (text_input_is_focused)
        {
            RenderTextCursor(scratch.arena, &App::app_state->input_box, App::app_state->fonts);
        }

        // if (App::app_state->hovered_highlight_root)
        // {
        //     Rng2F32 rect = App::app_state->highlight_rects[App::app_state->hovered_highlight_root].rect;
        //     F32 padding = 5;
        //     Rectangle r = {rect.x0 - padding, rect.y0 - padding,
        //                 (rect.x1 - rect.x0) + padding * 2,
        //                 (rect.y1 - rect.y0) + padding * 2};
        //     DrawRectangleRounded(r,
        //                 0.5f,
        //                 0,
        //                 {255, 180, 80, 80});
        //             DrawRectangleRoundedLinesEx(r, 0.5f, 0, 1, {255, 160, 40, 255});
        // }

        if (App::app_state->highlight_root)
        {

            expr::Node* hovered_root = App::app_state->hovered_box->node->highlight_root;
            U32 line = App::app_state->hovered_box->line_index;

            App::app_state->current_mark++;
            MarkSubTree(hovered_root);

            Rng2F32 result_rect{std::numeric_limits<F32>::max(), std::numeric_limits<F32>::max(), std::numeric_limits<F32>::min(), std::numeric_limits<F32>::min()};

            for (NodeBox& box : App::app_state->node_boxes)
            {
                if (box.line_index == line && 
                    box.node->visit_mark == App::app_state->current_mark)
                {
                    result_rect = RectUnion(result_rect, box.rect);
                }
            }

            F32 padding = 6;
            Rectangle r = {result_rect.x0-padding, result_rect.y0-padding, 
                (result_rect.x1 - result_rect.x0) + padding*2, 
                (result_rect.y1 - result_rect.y0) + padding*2};
            DrawRectangleRounded(r,
                0.5f,
                0,
                {255, 180, 80, 80});
            DrawRectangleRoundedLinesEx(r, 0.5f, 0, 1, {255, 160, 40, 255});
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

