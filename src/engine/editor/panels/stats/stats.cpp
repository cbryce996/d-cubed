#include "stats.h"

#include "editor/editor.h"
#include "imgui.h"

void Stats::draw (EditorContext& editor_context) {
	ImGui::SetNextWindowClass (editor_context.window);
	ImGui::Begin ("Stats", nullptr, ImGuiWindowFlags_NoCollapse);

	const ImGuiIO& io = ImGui::GetIO ();

	const float fps = io.Framerate;
	const float ms = (fps > 0.0f) ? (1000.0f / fps) : 0.0f;

	ImGui::Text ("FPS: %.1f", fps);
	ImGui::Text ("Frame: %.2f ms", ms);
	ImGui::End ();
}
