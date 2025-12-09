/*  date = December 09th 2025 06:15 PM */ 


struct AppState
{
	Arena* arena;
	// Per-frame scratch
	Arena* frame_arena;

	// clay
	Clay_RenderCommandArray render_commands;
	Font fonts[1];

	// TODO(me): cross-platform solution for paths, in windows msvc can embed into exe, in linux and mac can with #embed or can package into .App or .AppInstaller
	// paths
	// String8 data_directory_path
	// String8 font_path;

	// UI state

	// App config struct?
};

AppState* app_state = nullptr;

// Clay stuff
const int FONT_ID_BODY_16 = 0;
Clay_Color COLOR_WHITE = { 255, 255, 255, 255 }; 

void HandleClayErrors(Clay_ErrorData errorData) {
    // See the Clay_ErrorData struct for more information
    printf("%s", errorData.errorText.chars);
    // switch(errorData.errorType) {
    //     // etc
    // }
}

internal void App_Initialise();
internal void App_BuildUI();
internal void App_Shutdown();