#include "editor.h"

#include "render/buffers/buffer.h"

#include <imgui.h>
#include <imgui_internal.h>
#include <ranges>

void EditorManager::draw_entity_node (IEntity* entity, EditorState& state) {
	const bool is_leaf = entity->children.empty ();

	ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow
							   | ImGuiTreeNodeFlags_SpanAvailWidth;

	if (is_leaf) {
		flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
	}

	if (state.selected_entity == entity)
		flags |= ImGuiTreeNodeFlags_Selected;

	bool tree_pushed = false;

	if (is_leaf) {
		ImGui::TreeNodeEx (
			entity,
			flags | ImGuiTreeNodeFlags_Leaf
				| ImGuiTreeNodeFlags_NoTreePushOnOpen,
			"%s", entity->name.c_str ()
		);
	} else {
		tree_pushed = ImGui::TreeNodeEx (
			entity, flags, "%s", entity->name.c_str ()
		);
	}

	if (ImGui::IsItemClicked ()) {
		if (state.selected_entity != entity) {
			state.selected_entity = entity;
			state.cached_rotation_euler[entity] = glm::degrees (
				glm::eulerAngles (entity->transform.rotation)
			);
		}
	}

	if (tree_pushed) {
		for (auto* child : entity->children) {
			draw_entity_node (child, state);
		}
		ImGui::TreePop ();
	}
}

void EditorManager::draw_hierarchy (
	const ImGuiWindowClass& window_class, const RenderState& render_state,
	EditorState& editor_state
) {
	ImGui::SetNextWindowClass (&window_class);
	ImGui::Begin ("Hierarchy", nullptr, ImGuiWindowFlags_NoCollapse);

	Scene& scene = *render_state.scene;

	if (!editor_state.selected_entity) {
		for (const auto& entity : scene.scene_entities | std::views::values) {
			if (!entity->parent) {
				editor_state.selected_entity = entity.get ();
				break;
			}
		}
	}

	for (const auto& entity : scene.scene_entities | std::views::values) {
		if (!entity->parent) {
			draw_entity_node (entity.get (), editor_state);
		}
	}

	ImGui::End ();
}

void EditorManager::draw_inspector (
	const ImGuiWindowClass& window_class, EditorState& editor_state
) {
	ImGui::SetNextWindowClass (&window_class);
	ImGui::Begin ("Inspector", nullptr, ImGuiWindowFlags_NoCollapse);

	IEntity* entity = editor_state.selected_entity;

	if (!entity) {
		ImGui::TextDisabled ("No entity selected");
		ImGui::End ();
		return;
	}

	ImGui::Text ("Entity");
	ImGui::Separator ();

	char name_buffer[256];
	std::snprintf (
		name_buffer, sizeof (name_buffer), "%s", entity->name.c_str ()
	);
	if (ImGui::InputText ("Name", name_buffer, sizeof (name_buffer))) {
		entity->name = name_buffer;
	}

	ImGui::Spacing ();

	// Dummy transform data
	ImGui::Text ("Transform");
	ImGui::Separator ();

	Transform& transform = entity->transform;

	// Position & scale stay direct
	ImGui::DragFloat3 ("Position", glm::value_ptr (transform.position), 0.05f);
	ImGui::DragFloat3 ("Scale", glm::value_ptr (transform.scale), 0.05f);

	// --- Smooth rotation ---
	glm::vec3& euler = editor_state.cached_rotation_euler[entity];

	if (ImGui::DragFloat3 ("Rotation", glm::value_ptr (euler), 0.05f)) {
		transform.rotation = glm::quat (glm::radians (euler));
	}

	ImGui::End ();
}

void EditorManager::draw_console (const ImGuiWindowClass& window_class) {
	ImGui::SetNextWindowClass (&window_class);
	ImGui::Begin ("Console", nullptr, ImGuiWindowFlags_NoCollapse);
	ImGui::Text ("Logs...");
	ImGui::End ();
}

void EditorManager::draw_stats (const ImGuiWindowClass& window_class) {
	ImGui::SetNextWindowClass (&window_class);
	ImGui::Begin ("Stats", nullptr, ImGuiWindowFlags_NoCollapse);
	ImGui::Text ("FPS...");
	ImGui::End ();
}

void EditorManager::draw_main_menu () {
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

void EditorManager::draw_viewport (
	const BufferManager& buffer_manager, const ImGuiWindowClass& window_class
) {
	constexpr float TOOLBAR_HEIGHT = 32.0f;

	ImGui::SetNextWindowClass (&window_class);

	ImGui::PushStyleVar (ImGuiStyleVar_WindowPadding, ImVec2 (0, 0));
	ImGui::PushStyleVar (ImGuiStyleVar_ItemSpacing, ImVec2 (0, 0));

	ImGui::Begin (
		"Viewport", nullptr,
		ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse
			| ImGuiWindowFlags_NoCollapse
	);

	ImGui::TextColored (
		editor_mode == Editing ? ImVec4 (0, 1, 0, 1) : ImVec4 (1, 0.5f, 0, 1),
		editor_mode == Editing ? "EDITOR MODE" : "PLAY MODE"
	);

	const ImVec2 avail = ImGui::GetContentRegionAvail ();
	const int vw = static_cast<int> (avail.x);
	const int vh = static_cast<int> (avail.y);

	viewport_state.width = vw > 0 ? vw : 0;
	viewport_state.height = vh > 0 ? vh : 0;

	if (buffer_manager.viewport_texture && vw > 0 && vh > 0) {
		ImGui::Image (
			buffer_manager.viewport_texture, avail, ImVec2 (0, 0), ImVec2 (1, 1)
		);
	} else {
		ImGui::Text ("Viewport texture not ready...");
	}

	editor_state.viewport_hovered = ImGui::IsWindowHovered (
		ImGuiHoveredFlags_AllowWhenBlockedByPopup
	);

	editor_state.viewport_focused = ImGui::IsWindowFocused (
		ImGuiFocusedFlags_RootAndChildWindows
	);

	ImGui::End ();
	ImGui::PopStyleVar (2);
}

void EditorManager::create_ui (
	const BufferManager& buffer_manager, RenderState& render_state
) {
	constexpr ImGuiDockNodeFlags dock_flags
		= ImGuiDockNodeFlags_PassthruCentralNode;

	const ImGuiID dock_space_id = ImGui::DockSpaceOverViewport (
		ImGui::GetID ("MainDockspace"), ImGui::GetMainViewport (), dock_flags,
		nullptr
	);

	static bool built = false;
	if (!built) {
		layout_ui (dock_space_id);
		built = true;
	}

	ImGuiWindowClass docked_panel_class;
	docked_panel_class.DockNodeFlagsOverrideSet
		= ImGuiDockNodeFlags_NoWindowMenuButton;

	draw_viewport (buffer_manager, docked_panel_class);
	draw_main_menu ();
	draw_hierarchy (docked_panel_class, render_state, editor_state);
	draw_inspector (docked_panel_class, editor_state);
	draw_console (docked_panel_class);
	draw_stats (docked_panel_class);
}

void EditorManager::layout_ui (ImGuiID dock_base) {
	ImGui::DockBuilderRemoveNode (dock_base);
	ImGui::DockBuilderAddNode (dock_base, ImGuiDockNodeFlags_DockSpace);
	ImGui::DockBuilderSetNodeSize (
		dock_base, ImGui::GetMainViewport ()->WorkSize
	);

	ImGuiID dock_left = ImGui::DockBuilderSplitNode (
		dock_base, ImGuiDir_Left, 0.20f, nullptr, &dock_base
	);
	ImGuiID dock_right = ImGui::DockBuilderSplitNode (
		dock_base, ImGuiDir_Right, 0.25f, nullptr, &dock_base
	);
	ImGuiID dock_bottom = ImGui::DockBuilderSplitNode (
		dock_base, ImGuiDir_Down, 0.25f, nullptr, &dock_base
	);

	ImGuiID dock_stats = ImGui::DockBuilderSplitNode (
		dock_bottom, ImGuiDir_Right, 0.35f, nullptr, &dock_bottom
	);

	ImGui::DockBuilderDockWindow ("Viewport", dock_base);
	ImGui::DockBuilderDockWindow ("Hierarchy", dock_left);
	ImGui::DockBuilderDockWindow ("Inspector", dock_right);
	ImGui::DockBuilderDockWindow ("Console", dock_bottom);
	ImGui::DockBuilderDockWindow ("Stats", dock_stats);

	ImGui::DockBuilderFinish (dock_base);
}
