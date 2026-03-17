/*  date = December 09th 2025 06:15 PM */

// TODO(sb): make this iterative
internal String8List* BuildStringList(Arena* arena, expr::Node* node, String8List* list)
{
	
	using namespace expr;


	switch (node->kind)
	{
		case NodeKind::Number: {
			Str8ListPushF(arena, list, "%.2f", node->number);
		} break;
		case NodeKind::Variable: {
			goto err;
		} break;
		case NodeKind::UnaryOp: {
			goto err;
		} break;
		case NodeKind::BinaryOp : {
			if (node->bin_ops == BinOpKind::Minus)
			BuildStringList(arena, node->bin_left, list);
			Str8ListPushF(arena, list, "-");
			BuildStringList(arena, node->bin_right, list);
		} break;
		case NodeKind::NaryOp: {
			goto err;
		} break;
		default: 
			err:
			Assert(false && "unrecognised node kind");
	};
	
	return list;
}

internal String8 String8FromNode(Arena* arena, expr::Node* node)
{
	String8List list{};
	String8 result{};

	BuildStringList(arena, node, &list);
	result = Str8ListJoin(arena, &list);

	return result;

}

internal char* CstrFromNode(Arena* arena, expr::Node* node)
{
	ArenaTemp scratch = ScratchBegin(0, 0);

	String8 s = String8FromNode(scratch.arena, node);	
	char* result = CStrFromStr8(arena, s);
	ScratchEnd(scratch);
	return result;
}

internal Clay_Dimensions My_MeasureText(Clay_StringSlice text, Clay_TextElementConfig *config, void *userData) {
	ArenaTemp scratch = ScratchBegin(0, 0);
    U16 font_size = config->fontSize;
    if (font_size == 0)
    {
        font_size = 16; // default font size
    }

    // Font font = LoadFontEx(config->fontId, (int)font_size);
    char* str = PushArray(scratch.arena, char, text.length+1);
    MemoryCopy((void *)str, text.chars, text.length);
    str[text.length] = 0;

    Vector2 dims = MeasureTextEx(App::app_state->fonts[0], str, font_size, config->letterSpacing);

    ScratchEnd(scratch);

    return Clay_Dimensions{(float)dims.x, (float)dims.y};
}

internal void 
RenderTextCursor(Arena* arena, UI::TextEditState* state, Font* fonts)
{
	ArenaTemp scratch = ScratchBegin(&arena, 1);

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

	// char temp_buffer[1024*5];

	S64 cursor_pos = (S64)state->cursor;

	char* temp_buffer = PushArray(scratch.arena, char, cursor_pos);
	// if (cursor_pos > state->text.size)
	// 	cursor_pos = (S64)state->text.size;
	ClampTop(cursor_pos, (S64)state->text.size);

	memcpy(temp_buffer, state->text.str, cursor_pos);
	temp_buffer[cursor_pos] = '\0';

	Vector2 text_size = MeasureTextEx(font_to_use, temp_buffer, 24.0f, 0.0f);	//TODO(sb): pass the font size and letter spacing as a global variable so its the same

	// cursor doesn't blink on input events because blink timer is reset to 0.0 in main event loop
	App::app_state->blink_timer += GetFrameTime();

	if (fmod(App::app_state->blink_timer, 1.0) < 0.5)
	{
		F32 cursor_x = bbox.x + text_size.x + scroll_data.scrollPosition->x;
		DrawRectangle((int)(cursor_x), (int)bbox.y, 2, (int)bbox.height, {0, 0, 0, 255});
	}

	EndScissorMode();
	ScratchEnd(scratch);
}

internal B32 Contains2F32(Rng2F32 r, Vec2F32 p)
{
	B32 result = (p.x >= r.x0 && p.x <= r.x1 && p.y >= r.y0 && p.y <= r.y1);
	return result;
}

// internal NodeBox& FindBoxFromNode(expr::Node* node)
// {
// 	// for (auto& box : App::app_state->node_boxes)
// 	// {
// 	// 	if (node == box.node)
// 	// 		return box;
// 	// }
// 	// Assert(false && "no nodebox found for node");
// 	auto it = app_state->node_boxes_cache.find(node->id);	 //FIX THIS
//     Assert(it != app_state->node_boxes_cache.end() && "node not found in cache");
//     U32 node_boxes_index = it->second;
//     NodeBox& node_box = app_state->node_boxes[node_boxes_index];
// }

// internal void InsertNode(NodeBox* node_box, U64 key)
// {
// 	U32 index = App::app_state->node_boxes.size()-1;
// 	const bool ok = App::app_state->node_boxes_cache.insert({key, index}).second;
// 	Assert(ok && "insert node failed in InsertNode");
// }


// TODO(sb): incorporate step_index too or line_index
internal void EmitToken(expr::Node* node, char const* token_str, U32 step_index, F32 pad_left=0.0f, F32 pad_right=0.0f)
{
	// Add to nodebox to vector
	App::app_state->node_boxes.emplace_back(NodeBox{});
	NodeBox& box = App::app_state->node_boxes.back();
	box.line_index = step_index; // dummy value 
	// box.emit_count = emit_count;
	box.node = node;

	// InsertNode(&box);
	
	CustomLayoutElement* custom = PushStruct(App::app_state->frame_arena, CustomLayoutElement);

	custom->type = CUSTOM_LAYOUT_ELEMENT_TYPE_EXPR_NODE;
	custom->expr_node.node_boxes = &App::app_state->node_boxes;
	custom->expr_node.node = node;
	custom->expr_node.text = token_str;
	custom->expr_node.line_index = step_index;
	custom->expr_node.font_size = 24.0f;
	custom->expr_node.letter_spacing = 0.0f;
	custom->expr_node.font_id = 0;
	custom->expr_node.colour = Clay_Color{0, 0, 0,255};
	custom->expr_node.horizontal_padding = {pad_left, pad_right};
	// custom->expr_node.hovered_highlight_group = nullptr;
	// custom->expr_node.highlight_rects = &App::app_state->highlight_rects;
	custom->expr_node.node_box_index = App::app_state->node_boxes.size()-1;

	Vector2 size = MeasureTextEx(App::app_state->fonts[0], token_str, custom->expr_node.font_size, custom->expr_node.letter_spacing); // TODO(sb): hardcoded font id fix


	CLAY_AUTO_ID({
		.layout = {
			.sizing = {
				// TODO(sb): Different nodes have diff child gaps, like unary minus or power
				// Very hacky solution tho, it only adds a "gap" to the right, work on this.
				// even when moving to renderer, how do you change the x bounding box?
				.width = size.x + pad_left + pad_right,
				.height = size.y,
			},
			.childAlignment = {
				.x = CLAY_ALIGN_X_CENTER,
				.y = CLAY_ALIGN_Y_CENTER,
			},
		},
		.custom = {
			.customData = custom,
		}
	}) 
	{
		if (Clay_Hovered())
		{
			App::app_state->highlight_root = node->highlight_root;
			App::app_state->hovered_box = &box;
			// custom->expr_node.hovered_highlight_group = node->highlight_group;
		}
	};
}

internal void RenderNode(expr::Node* node, U32 step_index)
{
	using namespace expr;
	switch (node->kind)
	{
		case NodeKind::Number: {
			EmitToken(node, CstrFromNode(App::app_state->solutions_arena, node), step_index);
		} break;

		case NodeKind::Variable: {
			EmitToken(node, CStrFromStr8(App::app_state->solutions_arena, node->name), step_index);
		} break;

		case NodeKind::UnaryOp: {
			if (node->un_ops == UnOpKind::Negate)
			{
				EmitToken(node, "-", step_index);
				RenderNode(node->unary_child, step_index);
			}
			else
			{
				goto not_supported;
			}
		} break;

		case NodeKind::BinaryOp: {
			if (node->bin_ops == BinOpKind::Minus)
			{
				RenderNode(node->bin_left, step_index);
				EmitToken(node, "-", step_index, g::pad_left, g::pad_right);
				RenderNode(node->bin_right, step_index);
			}
			else
			{
				goto not_supported;
			}
		} break;

		case NodeKind::NaryOp: {
			Node* last{};
			if (node->nary_ops == NaryOpKind::Multiply)
			{
				for (Node* it = node->nary_first; 
					!NodeIsNil(it->nary_next); 
					it = it->nary_next)
				{
					RenderNode(it, step_index);
					EmitToken(node, "×", step_index, g::pad_left, g::pad_right);
					last = it->nary_next;
				}
				// last node
				RenderNode(last, step_index);
			}

			else
			{
				goto not_supported;
			}
			
		} break;

		default: 
			goto not_supported;
			not_supported:
				Assert(false && "node not supported");
	}
}


namespace App
{

internal void Initialise(Arena* arena)
{
	// initialise app state 
	app_state = PushArray(arena, State, 1);
	app_state->clay_arena = ArenaAlloc();
	app_state->string_arena = ArenaAlloc(true);
	app_state->solutions_arena = ArenaAlloc();
	app_state->frame_arena = ArenaAlloc();
	app_state->input_box_limit = INPUT_TEXT_OFFSET;
	app_state->placeholder_text = Str8Lit("3y = 2(x + 2)");

	// stl
	new(&app_state->node_boxes) std::vector<NodeBox>();
	// new(&app_state->node_boxes_cache) std::unordered_map<U64, U32>();

	// initialise clay and raylib
	EnableEventWaiting();
	Clay_Raylib_Initialize(1024, 768, "Introducing Clay Demo", FLAG_WINDOW_RESIZABLE | FLAG_MSAA_4X_HINT | FLAG_VSYNC_HINT); // Extra parameters to this function are new since the video was published
    SetTargetFPS(30);
	// Clay_SetMaxMeasureTextCacheWordCount(KiB(64));

	U64 min_memory_size = Clay_MinMemorySize();
	void* clay_mem = PushArrayNoZero(app_state->clay_arena, U8, min_memory_size);

	Clay_Arena clay_memory = Clay_CreateArenaWithCapacityAndMemory(min_memory_size, clay_mem);
	
	// loading entire Basic Latin block for Clay debug overlay, otherwise just pass g::codepoints
	constexpr int clay_codepoint_count = 128;
	constexpr int app_codepoint_count = ArrayCount(g::codepoints);
	int codepoints[clay_codepoint_count + app_codepoint_count]{};
	for(int i = 0; i < clay_codepoint_count; i++)
		codepoints[i] = i;

	for (int i = 0; i < app_codepoint_count; i++)
		codepoints[i + 128] = g::codepoints[i];
	Clay_Initialize(clay_memory, Clay_Dimensions{(float)GetScreenWidth(), (float)GetScreenHeight()}, Clay_ErrorHandler{HandleClayErrors});
	// TODO(me): embed textures, look at discord martins
	//NOTE(sb): giving absoloute path so that radbg can find it
    app_state->fonts[FONT_ID_BODY_16] = LoadFontEx("D:\\Coding\\dsp-project-refactor\\data\\Roboto-Regular.ttf", 48, codepoints, ArrayCount(codepoints)); 

    SetTextureFilter(app_state->fonts[FONT_ID_BODY_16].texture, TEXTURE_FILTER_BILINEAR);
    Clay_SetMeasureTextFunction(My_MeasureText, app_state->fonts);

    // debug tools
    Clay_SetDebugModeEnabled(true);

    // text widget
    app_state->input_box = zero_struct;
    app_state->input_box.text.str = PushArrayNoZero(app_state->string_arena, U8, INPUT_TEXT_OFFSET);
    app_state->input_box.cursor = 0;
    app_state->input_box.mark = 0;
    app_state->input_box.scroll_offset_x = 0.0f;

}

// internal 

// internal void EmitNode(Node* n, U64 line_index, Clay_TextElementConfig* text_config)
// {
// 	NodeRenderPayload* payload = PushStruct(App::app_state->frame_arena, NodeRenderPayload);
// 	payload->node = n;
// 	payload->line_index = line_index;

// 	CLAY({
// 		.custom = {
// 			.customData = payload;
// 		}
// 	}) {
// 		CLAY_TEXT(NodeToString(node), text_config);
// 	}
// }

// internal void EmitOperatorToken(Node* op_node, U64 line_index, Clay_TextElementConfig* text_config)
// {
// 	NodeRenderPayload* payload = PushStruct(App::app_state->frame_arena, NodeRenderPayload);
// 	payload->node = op_node;
// 	payload->line_index = line_index;

// 	CLAY({
// 		.custom = {
// 			.customData = payload;
// 		}
// 	}) {
// 		CLAY_TEXT(NodeKindToString(node->kind), text_config);
// 	}
// }

// internal void EmitTree(Node* n, U64 line_index, Clay_TextElementConfig* text_config)
// {
// 	switch (n->kind)
// 	{
// 		case expr::NodeKind::NaryOp: {
// 			for (Node* it = n->nary_first; !NodeIsNil(it); it = it->nary_next)
// 			{
// 				EmitTree(it, line_index, text_config);
// 				if (!NodeIsNil(it->nary_next))
// 					EmitOperatorToken(n, line_index, text_config);	// emitting op between children
// 			}
// 		} break;
// 		default:
// 			EmitNode(n, line_index);
// 	}
// }

	

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
// -------------------------------------------------------------------------------------------------
			//- OUTPUT_WINDOW
			CLAY(CLAY_ID("OUTPUT_WINDOW"), {
				.layout = {
					.sizing = layout_expand,
					.childAlignment = {
						.x = CLAY_ALIGN_X_CENTER,
						.y = CLAY_ALIGN_Y_CENTER,
					},
					.layoutDirection = CLAY_TOP_TO_BOTTOM,
				}
			})
			//- OUTPUT_WINDOW Children
			{
// -------------------------------------------------------------------------------------------------
				if (App::app_state->root_node)
				{
					App::app_state->node_boxes.clear();
            		// App::app_state->node_boxes_cache.clear();
            		for (U32 step{}; step < 2; ++step)
            		{
						CLAY_AUTO_ID({
							.layout = {
								.sizing = { .width = CLAY_SIZING_GROW(), .height = 50},
								.childAlignment = {
									.x = CLAY_ALIGN_X_CENTER,
									.y = CLAY_ALIGN_Y_CENTER,
								},

								// .childGap = g::OUTPUT_LINE_DEFAULT_CHILD_GAP,
								.layoutDirection = CLAY_LEFT_TO_RIGHT,
							}
						})
						{
							RenderNode(App::app_state->root_node, step); // NOTE(sb): the line_indez is hard coded but in the for loop it will be i
						}
					}
				}
			}

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
					.childOffset = Clay_GetScrollOffset(),
				},
				.border = {
					.color = {217, 218, 223, 255},	// greyish
					.width = {1, 1, 1, 1},
				}
			}) 
			//- INPUT_BOX Children
			{
// -------------------------------------------------------------------------------------------------
				// TODO(sb): add padding to text so it doesnt touch edges, then make sure to compensate for the padding when rendering cursor
				Clay_String input_buffer = ClayStringFromString8(app_state->placeholder_text);
				Clay_Color input_colour = {0, 0, 0, 70};//{211, 211, 211, 255};

				if (!App::app_state->need_placeholder)
				{
					input_buffer = ClayStringFromString8(app_state->input_box.text);
					input_colour = {0, 0, 0, 255};
				}

				CLAY_TEXT(input_buffer, CLAY_TEXT_CONFIG({
                    .textColor = input_colour,
                    .fontId = FONT_ID_BODY_16,
                    .fontSize = 24,
	                }));

				// TODO(sb): Custom text element to avoid clay's text measurement cache
				// CustomLayoutElement input_element = {	// YOU WERE HERE
				// 	.type = CUSTOM_LAYOUT_ELEMENT_TYPE_TEXT_INPUT,
				// 	.customData = {

				// 	}
				// }

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
		//- SCROLLBAR START
		#if 0
		Clay_ScrollContainerData scroll_data = Clay_GetScrollContainerData(Clay_GetElementId(CLAY_STRING("INPUT_BOX")));
		// scroll
		if (scroll_data.found)
		{
			Clay_ElementDeclaration d{};
			d.floating.attachTo = CLAY_ATTACH_TO_ELEMENT_WITH_ID;
			d.floating.zIndex = 1;
			d.floating.parentId = Clay_GetElementId(CLAY_STRING("INPUT_BOX")).id;
			d.floating.attachPoints.element = CLAY_ATTACH_POINT_LEFT_BOTTOM;
			d.floating.attachPoints.parent = CLAY_ATTACH_POINT_LEFT_BOTTOM;
			d.floating.offset = Clay_Vector2{-(scroll_data.scrollPosition->x / scroll_data.contentDimensions.width) * scroll_data.scrollContainerDimensions.width, 0};

			// CLAY(CLAY_ID("SCROLLBAR"), {
			// 	.floating = {
			// 		.attachTo = CLAY_ATTACH_TO_ELEMENT_WITH_ID,
			// 		.offset = { .y = -(scroll_data.scrollPosition->y / scroll_data.contentDimensions.height) * scroll_data.scrollContainerDimensions.height },
			// 		.zIndex = 1,
			// 		.parentId = Clay_GetElementId(CLAY_STRING("INPUT_BOX")).id,
			// 		.attachPoints = ( .element = CLAY_ATTATCH_POINT_RIGHT_TOP, .parent = CLAY_ATTATCH_POINT_RIGHT_TOP )
			// 	}
			// })

			CLAY(CLAY_ID("SCROLLBAR"), d)
			//- SCROLLBAR Children
			{
// -------------------------------------------------------------------------------------------------
				//- SCROLLBAR_BUTTON START
				Clay_ElementDeclaration e{};
				e.layout.sizing = Clay_Sizing{CLAY_SIZING_FIXED((scroll_data.scrollContainerDimensions.height / scroll_data.contentDimensions.height) * scroll_data.scrollContainerDimensions.height), CLAY_SIZING_FIXED(12)};
				e.backgroundColor = Clay_PointerOver(Clay_GetElementId(CLAY_STRING("SCROLLBAR"))) ? Clay_Color{100, 100, 140, 150} : Clay_Color{120, 120, 160, 150};
				e.cornerRadius = CLAY_CORNER_RADIUS(6);
				CLAY(CLAY_ID("SCROLLBAR_BUTTON"), e)
				{};
				//- SCROLLBAR_BUTTON END
// -------------------------------------------------------------------------------------------------
			};
// -------------------------------------------------------------------------------------------------
		}
		#endif
	};
	//- Root End
// -------------------------------------------------------------------------------------------------

}

internal void Shutdown()
{
	// TODO(sb): see if its even worth callnig destructors on program close up, why don't we just let the OS claim that memory back
	app_state->node_boxes.~vector();
	// app_state->node_boxes_cache.~unordered_map();
	// app_state->highlight_rects.~unordered_map();
	Clay_Raylib_Close();
}

} // namespace App