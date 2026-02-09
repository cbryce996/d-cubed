#include "editor.h"

#include "render/buffers/buffer.h"
#include "render/resources/resources.h"

#include <imgui.h>
#include <imgui_internal.h>
#include <ranges>

EditorManager::EditorManager (const ViewportState viewport_state)
	: viewport_state (viewport_state) {}

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
	const ResourceManager& resource_manager,
	const ImGuiWindowClass& window_class
) {
	ImGui::SetNextWindowClass (&window_class);

	ImGui::PushStyleVar (ImGuiStyleVar_WindowPadding, ImVec2 (0, 0));
	ImGui::PushStyleVar (ImGuiStyleVar_ItemSpacing, ImVec2 (0, 0));

	const bool open = ImGui::Begin (
		"Viewport", nullptr,
		ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse
			| ImGuiWindowFlags_NoCollapse
	);

	auto end_scope = [&] {
		ImGui::End ();
		ImGui::PopStyleVar (2);
	};

	if (!open) {
		end_scope ();
		return;
	}

	SDL_GPUTexture* tex = resource_manager.viewport_target.read ();
	if (!tex) {
		ImGui::Text ("Viewport texture not ready...");
		end_scope ();
		return;
	}

	const ImVec2 win_pos = ImGui::GetWindowPos ();
	const ImVec2 cr_min = ImGui::GetWindowContentRegionMin ();
	const ImVec2 cr_max = ImGui::GetWindowContentRegionMax ();

	const ImVec2 inner_min = ImVec2 (
		win_pos.x + cr_min.x, win_pos.y + cr_min.y
	);
	const ImVec2 inner_max = ImVec2 (
		win_pos.x + cr_max.x, win_pos.y + cr_max.y
	);
	const ImVec2 inner_size = ImVec2 (
		inner_max.x - inner_min.x, inner_max.y - inner_min.y
	);

	if (inner_size.x <= 1.0f || inner_size.y <= 1.0f) {
		end_scope ();
		return;
	}

	const float tex_w = (float)resource_manager.viewport_target.width;
	const float tex_h = (float)resource_manager.viewport_target.height;
	const float tex_aspect = tex_w / tex_h;
	const float panel_aspect = inner_size.x / inner_size.y;

	// UV crop (centered)
	ImVec2 uv0 (0.0f, 0.0f);
	ImVec2 uv1 (1.0f, 1.0f);

	if (panel_aspect < tex_aspect) {
		// panel is narrower -> crop horizontally
		const float frac = panel_aspect
						   / tex_aspect; // visible fraction of width
		const float u0 = (1.0f - frac) * 0.5f;
		uv0.x = u0;
		uv1.x = u0 + frac;
	} else if (panel_aspect > tex_aspect) {
		// panel is wider -> crop vertically (rare if you always "fill Y", but
		// keeps it robust)
		const float frac = tex_aspect
						   / panel_aspect; // visible fraction of height
		const float v0 = (1.0f - frac) * 0.5f;
		uv0.y = v0;
		uv1.y = v0 + frac;
	}

	ImDrawList* dl = ImGui::GetWindowDrawList ();
	dl->PushClipRect (inner_min, inner_max, true);

	dl->AddRectFilled (inner_min, inner_max, IM_COL32 (10, 10, 10, 255));

	// Draw EXACTLY in the panel rect, aligned, and crop via UVs
	dl->AddImage ((ImTextureID)tex, inner_min, inner_max, uv0, uv1);

	// Overlay anchored to the visible panel
	dl->AddText (
		ImVec2 (inner_min.x + 10.0f, inner_min.y + 10.0f),
		IM_COL32 (0, 255, 0, 255),
		(editor_mode == Editing) ? "EDITOR MODE" : "PLAY MODE"
	);

	dl->PopClipRect ();

	editor_state.viewport_hovered = ImGui::IsWindowHovered (
		ImGuiHoveredFlags_AllowWhenBlockedByPopup
	);
	editor_state.viewport_focused = ImGui::IsWindowFocused (
		ImGuiFocusedFlags_RootAndChildWindows
	);

	end_scope ();
}

void EditorManager::create_ui (
	const ResourceManager& resource_manager, RenderState& render_state
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

	draw_main_menu ();
	draw_hierarchy (docked_panel_class, render_state, editor_state);
	draw_inspector (docked_panel_class, editor_state);
	draw_console (docked_panel_class);
	draw_stats (docked_panel_class);
	draw_viewport (resource_manager, docked_panel_class);
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
