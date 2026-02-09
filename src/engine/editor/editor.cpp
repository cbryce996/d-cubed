#include "editor.h"

#include "render/buffers/buffer.h"
#include "render/resources/resources.h"

#include <imgui.h>
#include <imgui_internal.h>
#include <ranges>

void EditorManager::draw_entity_node (IEntity* entity, EditorState& state) {
	const bool is_leaf = entity->children.empty ();

	ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow
							   | ImGuiTreeNodeFlags_SpanFullWidth
							   | ImGuiTreeNodeFlags_FramePadding;

	if (is_leaf) {
		flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
	}

	ImGui::PushStyleVar (ImGuiStyleVar_IndentSpacing, 8.0f);

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

	ImGui::PopStyleVar ();
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

	// Tighter look just for this window
	ImGui::PushStyleVar (ImGuiStyleVar_FramePadding, ImVec2 (6, 3));
	ImGui::PushStyleVar (ImGuiStyleVar_ItemSpacing, ImVec2 (6, 4));

	ImGui::Begin ("Console", nullptr, ImGuiWindowFlags_NoCollapse);

	// Pull SDL errors into console (one-shot)
	if (const char* err = SDL_GetError (); err && err[0] != '\0') {
		console_entries.push_back (
			{IM_COL32 (255, 90, 90, 255), std::string ("[sdl] ") + err}
		);
		SDL_ClearError ();
	}

	// No border: use false for the "border" arg
	ImGui::BeginChild (
		"##console_scroller", ImVec2 (0, 0), false, ImGuiWindowFlags_NoScrollbar
	);

	// Slightly tighter line spacing
	ImGui::PushStyleVar (ImGuiStyleVar_ItemSpacing, ImVec2 (4, 2));

	for (const auto& e : console_entries) {
		ImGui::PushStyleColor (ImGuiCol_Text, e.color);
		ImGui::TextUnformatted (e.text.c_str ());
		ImGui::PopStyleColor ();
	}

	ImGui::PopStyleVar ();

	if (console_autoscroll
		&& ImGui::GetScrollY () >= ImGui::GetScrollMaxY () - 2.0f) {
		ImGui::SetScrollHereY (1.0f);
	}

	ImGui::EndChild ();
	ImGui::End ();

	ImGui::PopStyleVar (2);
}

void EditorManager::draw_stats (const ImGuiWindowClass& window_class) {
	ImGui::SetNextWindowClass (&window_class);
	ImGui::Begin ("Stats", nullptr, ImGuiWindowFlags_NoCollapse);

	const ImGuiIO& io = ImGui::GetIO ();

	const float fps = io.Framerate;
	const float ms = (fps > 0.0f) ? (1000.0f / fps) : 0.0f;

	ImGui::Text ("FPS: %.1f", fps);
	ImGui::Text ("Frame: %.2f ms", ms);
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

	const ImVec2 window_pos = ImGui::GetWindowPos ();
	const ImVec2 content_region_min = ImGui::GetWindowContentRegionMin ();
	const ImVec2 content_region_max = ImGui::GetWindowContentRegionMax ();

	const auto panel_min = ImVec2 (
		window_pos.x + content_region_min.x, window_pos.y + content_region_min.y
	);
	const auto panel_max = ImVec2 (
		window_pos.x + content_region_max.x, window_pos.y + content_region_max.y
	);
	const auto panel_size = ImVec2 (
		panel_max.x - panel_min.x, panel_max.y - panel_min.y
	);

	if (panel_size.x <= 1.0f || panel_size.y <= 1.0f) {
		end_scope ();
		return;
	}

	const float texture_ratio
		= static_cast<float> (resource_manager.viewport_target.width)
		  / static_cast<float> (resource_manager.viewport_target.height);

	const float panel_ratio = panel_size.x / panel_size.y;

	ImVec2 uv0 (0.0f, 0.0f);
	ImVec2 uv1 (1.0f, 1.0f);

	if (panel_ratio < texture_ratio) {
		const float frac = panel_ratio / texture_ratio;
		const float u0 = (1.0f - frac) * 0.5f;
		uv0.x = u0;
		uv1.x = u0 + frac;
	} else if (panel_ratio > texture_ratio) {
		const float frac = texture_ratio / panel_ratio;
		const float v0 = (1.0f - frac) * 0.5f;
		uv0.y = v0;
		uv1.y = v0 + frac;
	}

	ImDrawList* dl = ImGui::GetWindowDrawList ();
	dl->PushClipRect (panel_min, panel_max, true);

	dl->AddRectFilled (panel_min, panel_max, IM_COL32 (10, 10, 10, 255));

	dl->AddImage (tex, panel_min, panel_max, uv0, uv1);

	{
		constexpr float pad = 10.0f;
		constexpr float box_pad = 6.0f;

		const char* mode_text = (editor_mode == Editing) ? "EDITOR MODE"
														 : "PLAY MODE";
		const ImU32 mode_col = (editor_mode == Editing)
								   ? IM_COL32 (0, 255, 0, 255)	  // green
								   : IM_COL32 (255, 165, 0, 255); // orange

		const ImVec2 text_size = ImGui::CalcTextSize (mode_text);
		const ImVec2 text_pos (panel_min.x + pad, panel_min.y + pad);

		const ImVec2 box_min (text_pos.x - box_pad, text_pos.y - box_pad);
		const ImVec2 box_max (
			text_pos.x + text_size.x + box_pad,
			text_pos.y + text_size.y + box_pad
		);

		dl->AddRectFilled (box_min, box_max, IM_COL32 (30, 30, 30, 80), 4.0f);
		dl->AddText (text_pos, mode_col, mode_text);
	}

	{
		constexpr float pad = 10.0f;
		constexpr float box_pad = 6.0f;

		const ImGuiIO& io = ImGui::GetIO ();
		const ImVec2 fb = io.DisplayFramebufferScale;

		const int tex_w = resource_manager.viewport_target.width;
		const int tex_h = resource_manager.viewport_target.height;

		char buf[256];
		std::snprintf (
			buf, sizeof (buf),
			"Render: %dx%d\nPanel:  %.0fx%.0f\nFB:     %.2fx%.2f", tex_w, tex_h,
			panel_size.x, panel_size.y, fb.x, fb.y
		);

		const ImVec2 text_size = ImGui::CalcTextSize (buf);
		const ImVec2 text_pos (
			panel_max.x - pad - text_size.x, panel_min.y + pad
		);

		const ImVec2 box_min (text_pos.x - box_pad, text_pos.y - box_pad);
		const ImVec2 box_max (
			text_pos.x + text_size.x + box_pad,
			text_pos.y + text_size.y + box_pad
		);

		dl->AddRectFilled (box_min, box_max, IM_COL32 (30, 30, 30, 80), 4.0f);
		dl->AddText (text_pos, IM_COL32 (255, 255, 0, 255), buf);
	}

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
