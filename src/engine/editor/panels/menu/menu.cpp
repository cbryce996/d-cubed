#include "menu.h"

#include "imgui.h"

void Menu::draw (EditorContext& editor_context) {
	if (ImGui::BeginMainMenuBar ()) {
		if (ImGui::BeginMenu ("File")) {
			ImGui::MenuItem ("New");
			ImGui::MenuItem ("Open");
			ImGui::MenuItem ("Save");
			ImGui::EndMenu ();
		}
		if (ImGui::BeginMenu ("Engine")) {
			ImGui::MenuItem ("Reload Shaders");
			ImGui::EndMenu ();
		}
		ImGui::EndMainMenuBar ();
	}
}
