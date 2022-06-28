#include "engine.h"

void engine::quitConf( bool *open ) {
	if ( *open ) {
		ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration;
		// create centered window
		ImGui::SetNextWindowPos( ImVec2( totalScreenWidth / 2 - 120, totalScreenHeight / 2 - 25 ) );
		ImGui::SetNextWindowSize( ImVec2( 230, 55 ) );
		ImGui::Begin( "quit", open, flags );
		ImGui::Text( "Are you sure you want to quit?" );
		ImGui::Text( "  " );
		ImGui::SameLine();
		// button to cancel -> set this window's bool to false
		if ( ImGui::Button( " Cancel " ) )
			*open = false;
		ImGui::SameLine();
		ImGui::Text("      ");
		ImGui::SameLine();
		// button to quit -> set pquit to true
		if ( ImGui::Button( " Quit " ) )
			pQuit = true;
		ImGui::End();
	}
}

// static void HelpMarker( const char *desc ) {
// 	ImGui::TextDisabled( "(?)" );
// 	if ( ImGui::IsItemHovered() ) {
// 		ImGui::BeginTooltip();
// 		ImGui::PushTextWrapPos( ImGui::GetFontSize() * 35.0f );
// 		ImGui::TextUnformatted( desc );
// 		ImGui::PopTextWrapPos();
// 		ImGui::EndTooltip();
// 	}
// }


void engine::showControlsWindow(){
	// ImGui::Begin( "Controls", NULL, 0 );
	// unsigned int min = 1;
	// unsigned int max = 256;
	// ImGui::SliderScalar( "Display Width", ImGuiDataType_U32, &buffer.dimensions.x, &min, &max );
	// ImGui::SliderScalar( "Display Height", ImGuiDataType_U32, &buffer.dimensions.y, &min, &max );
	// ImGui::Text( " " );
	// ImGui::SliderScalar( "Buffer Width", ImGuiDataType_U32, &buffer.bufferSize.x, &min, &max );
	// ImGui::SliderScalar( "Buffer Height", ImGuiDataType_U32, &buffer.bufferSize.y, &min, &max );
	// ImGui::Text( " " );
	// int min2 = -256;
	// int max2 = 256;
	// ImGui::SliderScalar( "Offset X", ImGuiDataType_S32, &buffer.offset.x, &min2, &max2 );
	// ImGui::SliderScalar( "Offset Y", ImGuiDataType_S32, &buffer.offset.y, &min2, &max2 );
	// ImGui::End();
}

void engine::drawTextEditor() {
	ImGui::Begin( "Editor", NULL, 0 );
	static TextEditor editor;
	// static auto lang = TextEditor::LanguageDefinition::CPlusPlus();
	static auto lang = TextEditor::LanguageDefinition::GLSL();
	editor.SetLanguageDefinition( lang );

	auto cpos = editor.GetCursorPosition();
	// editor.SetPalette(TextEditor::GetLightPalette());
	editor.SetPalette( TextEditor::GetDarkPalette() );
	// editor.SetPalette(TextEditor::GetRetroBluePalette());

	static const char *fileToEdit = "resources/engineCode/shaders/blit.vs.glsl";
	std::ifstream t( fileToEdit );
	static bool loaded = false;
	if ( !loaded ) {
		editor.SetLanguageDefinition( lang );
		if ( t.good() ) {
			editor.SetText( std::string( ( std::istreambuf_iterator<char>(t)), std::istreambuf_iterator< char >() ) );
			loaded = true;
		}
	}

	// add dropdown for different shaders?
	ImGui::Text( "%6d/%-6d %6d lines  | %s | %s | %s | %s", cpos.mLine + 1,
							cpos.mColumn + 1, editor.GetTotalLines(),
							editor.IsOverwrite() ? "Ovr" : "Ins",
							editor.CanUndo() ? "*" : " ",
							editor.GetLanguageDefinition().mName.c_str(), fileToEdit );

	editor.Render( "Editor" );
	ImGui::End();
}

void engine::imguiFrameStart() {
	// Start the Dear ImGui frame
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplSDL2_NewFrame( window );
	ImGui::NewFrame();
	}

	void engine::imguiFrameEnd() {
	// get it ready to put on the screen
	ImGui::Render();

	// put imgui data into the framebuffer
	ImGui_ImplOpenGL3_RenderDrawData( ImGui::GetDrawData() );

	// platform windows ( pop out windows )
	ImGuiIO &io = ImGui::GetIO();
	if ( io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable ) {
		SDL_Window* backup_current_window = SDL_GL_GetCurrentWindow();
		SDL_GLContext backup_current_context = SDL_GL_GetCurrentContext();
		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault();
		SDL_GL_MakeCurrent( backup_current_window, backup_current_context );
	}
}
