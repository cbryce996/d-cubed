#include "console.h"

#include <SDL3/SDL_error.h>

#include "editor/editor.h"
#include "entity/entity.h"
#include "imgui.h"

void Console::draw (EditorContext& editor_context) {
	ImGui::SetNextWindowClass (editor_context.window);

	ImGui::PushStyleVar (ImGuiStyleVar_FramePadding, ImVec2 (6, 3));
	ImGui::PushStyleVar (ImGuiStyleVar_ItemSpacing, ImVec2 (6, 4));

	ImGui::Begin ("Console", nullptr, ImGuiWindowFlags_NoCollapse);

	if (const char* err = SDL_GetError (); err && err[0] != '\0') {
		console_entries.push_back (
			{IM_COL32 (255, 90, 90, 255), std::string ("[sdl] ") + err}
		);
		SDL_ClearError ();
	}

	ImGui::BeginChild (
		"##console_scroller", ImVec2 (0, 0), false, ImGuiWindowFlags_NoScrollbar
	);

	ImGui::PushStyleVar (ImGuiStyleVar_ItemSpacing, ImVec2 (4, 2));

	for (const auto& e : console_entries) {
		ImGui::PushStyleColor (ImGuiCol_Text, e.color);
		ImGui::TextUnformatted (e.text.c_str ());
		ImGui::PopStyleColor ();
	}

	ImGui::PopStyleVar ();

	ImGui::SetScrollHereY (1.0f);

	ImGui::EndChild ();
	ImGui::End ();

	ImGui::PopStyleVar (2);
}