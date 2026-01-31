#include "editor.h"

#include "render/buffers/buffer.h"

#include <imgui.h>
#include <imgui_internal.h>
#include <ranges>

void EditorManager::draw_entity_node (IEntity* entity, EditorState& state) {
	ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow
							   | ImGuiTreeNodeFlags_SpanAvailWidth;

	if (state.selected_entity == entity)
		flags |= ImGuiTreeNodeFlags_Selected;

	const bool opened = ImGui::TreeNodeEx (
		entity, flags, "%s", entity->name.c_str ()
	);

	if (ImGui::IsItemClicked ()) {
		state.selected_entity = entity;
	}

	if (opened) {
		for (auto* child : entity->children) {
			draw_entity_node (child, state);
		}
		ImGui::TreePop ();
	}
}

void EditorManager::draw_hierarchy(
	const ImGuiWindowClass& window_class, const RenderState& render_state,
	EditorState& editor_state
) {
	ImGui::SetNextWindowClass(&window_class);
	ImGui::Begin("Hierarchy", nullptr, ImGuiWindowFlags_NoCollapse);

	Scene& scene = *render_state.scene;

	if (!editor_state.selected_entity) {
		for (const auto& entity : scene.scene_entities | std::views::values) {
			if (!entity->parent) {
				editor_state.selected_entity = entity.get();
				break;
			}
		}
	}

	for (auto& entity : scene.scene_entities | std::views::values) {
		draw_entity_node(entity.get(), editor_state);
	}

	ImGui::End();
}

void EditorManager::draw_inspector(
	const ImGuiWindowClass& window_class, const EditorState& editor_state
) {
	ImGui::SetNextWindowClass(&window_class);
	ImGui::Begin("Inspector", nullptr, ImGuiWindowFlags_NoCollapse);

	IEntity* entity = editor_state.selected_entity;

	if (!entity) {
		ImGui::TextDisabled("No entity selected");
		ImGui::End();
		return;
	}

	ImGui::Text("Entity");
	ImGui::Separator();

	// Name
	char name_buffer[256];
	std::snprintf(name_buffer, sizeof(name_buffer), "%s", entity->name.c_str());
	if (ImGui::InputText("Name", name_buffer, sizeof(name_buffer))) {
		entity->name = name_buffer;
	}

	ImGui::Spacing();

	// Dummy transform data
	ImGui::Text("Transform");
	ImGui::Separator();

	static float position[3] = { 0.f, 0.f, 0.f };
	static float rotation[3] = { 0.f, 0.f, 0.f };
	static float scale[3]    = { 1.f, 1.f, 1.f };

	ImGui::DragFloat3("Position", position, 0.1f);
	ImGui::DragFloat3("Rotation", rotation, 0.1f);
	ImGui::DragFloat3("Scale",    scale,    0.1f);

	ImGui::End();
}

void EditorManager::draw_console(const ImGuiWindowClass& window_class) {
	ImGui::SetNextWindowClass(&window_class);
	ImGui::Begin("Console", nullptr, ImGuiWindowFlags_NoCollapse);
	ImGui::Text("Logs...");
	ImGui::End();
}

void EditorManager::draw_stats(const ImGuiWindowClass& window_class) {
	ImGui::SetNextWindowClass(&window_class);
	ImGui::Begin("Stats", nullptr, ImGuiWindowFlags_NoCollapse);
	ImGui::Text("FPS...");
	ImGui::End();
}

void EditorManager::draw_main_menu() {
	if (ImGui::BeginMainMenuBar()) {
		if (ImGui::BeginMenu("File")) {
			ImGui::MenuItem("New");
			ImGui::MenuItem("Open");
			ImGui::MenuItem("Save");
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Engine")) {
			ImGui::MenuItem("Reload Shaders");
			ImGui::EndMenu();
		}
		ImGui::EndMainMenuBar();
	}
}

void EditorManager::draw_viewport(
	const BufferManager& buffer_manager,
	const ImGuiWindowClass& window_class
) {
	ImGui::SetNextWindowClass(&window_class);

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));

	ImGui::Begin(
		"Viewport",
		nullptr,
		ImGuiWindowFlags_NoScrollbar |
		ImGuiWindowFlags_NoScrollWithMouse |
		ImGuiWindowFlags_NoCollapse
	);

	const ImVec2 avail = ImGui::GetContentRegionAvail();
	const int vw = static_cast<int>(avail.x);
	const int vh = static_cast<int>(avail.y);

	viewport_state.width  = vw > 0 ? vw : 0;
	viewport_state.height = vh > 0 ? vh : 0;

	if (buffer_manager.viewport_texture && vw > 0 && vh > 0) {
		ImGui::Image(
			buffer_manager.viewport_texture,
			avail,
			ImVec2(0, 0),
			ImVec2(1, 1)
		);
	} else {
		ImGui::Text("Viewport texture not ready...");
	}

	ImGui::End();
	ImGui::PopStyleVar(2);
}

void EditorManager::create_ui(
	const BufferManager& buffer_manager,
	RenderState& render_state
) {
	constexpr ImGuiDockNodeFlags dock_flags =
		ImGuiDockNodeFlags_PassthruCentralNode;

	const ImGuiID dock_space_id = ImGui::DockSpaceOverViewport(
		ImGui::GetID("MainDockspace"),
		ImGui::GetMainViewport(),
		dock_flags,
		nullptr
	);

	static bool built = false;
	if (!built) {
		layout_ui(dock_space_id);
		built = true;
	}

	ImGuiWindowClass docked_panel_class;
	docked_panel_class.DockNodeFlagsOverrideSet =
		ImGuiDockNodeFlags_NoWindowMenuButton;

	draw_viewport(buffer_manager, docked_panel_class);
	draw_main_menu();
	draw_hierarchy(docked_panel_class, render_state, editor_state);
	draw_inspector(docked_panel_class, editor_state);
	draw_console(docked_panel_class);
	draw_stats(docked_panel_class);
}

void EditorManager::layout_ui(ImGuiID dock_base) {
	ImGui::DockBuilderRemoveNode(dock_base);
	ImGui::DockBuilderAddNode(dock_base, ImGuiDockNodeFlags_DockSpace);
	ImGui::DockBuilderSetNodeSize(
		dock_base,
		ImGui::GetMainViewport()->WorkSize
	);

	ImGuiID dock_left = ImGui::DockBuilderSplitNode(
		dock_base, ImGuiDir_Left, 0.20f, nullptr, &dock_base
	);
	ImGuiID dock_right = ImGui::DockBuilderSplitNode(
		dock_base, ImGuiDir_Right, 0.25f, nullptr, &dock_base
	);
	ImGuiID dock_bottom = ImGui::DockBuilderSplitNode(
		dock_base, ImGuiDir_Down, 0.25f, nullptr, &dock_base
	);

	ImGuiID dock_stats = ImGui::DockBuilderSplitNode(
		dock_bottom, ImGuiDir_Right, 0.35f, nullptr, &dock_bottom
	);

	ImGui::DockBuilderDockWindow("Viewport", dock_base);
	ImGui::DockBuilderDockWindow("Hierarchy", dock_left);
	ImGui::DockBuilderDockWindow("Inspector", dock_right);
	ImGui::DockBuilderDockWindow("Console", dock_bottom);
	ImGui::DockBuilderDockWindow("Stats", dock_stats);

	ImGui::DockBuilderFinish(dock_base);
}
