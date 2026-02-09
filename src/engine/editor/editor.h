#ifndef EDITOR_H
#define EDITOR_H

#include "render/resources/resources.h"

#include <glm/glm.hpp>
#include <imgui.h>
#include <unordered_map>

struct RenderState;
class BufferManager;
class IEntity;

struct ViewportState {
	int width = 0;
	int height = 0;
};

struct EditorState {
	IEntity* selected_entity = nullptr;
	bool show_scene_graph = true;
	std::unordered_map<IEntity*, glm::vec3> cached_rotation_euler;

	bool viewport_hovered = false;
	bool viewport_focused = false;
};

enum EditorMode { Editing, Running };

class EditorManager {
  public:
	void draw_hierarchy (
		const ImGuiWindowClass& window_class, const RenderState& render_state,
		EditorState& editor_state
	);
	void draw_entity_node (IEntity* entity, EditorState& state);
	void draw_inspector (
		const ImGuiWindowClass& window_class, EditorState& editor_state
	);
	void draw_console (const ImGuiWindowClass& window_class);
	void draw_stats (const ImGuiWindowClass& window_class);

	void draw_main_menu ();
	void draw_viewport (
		const ResourceManager& resource_manager,
		const ImGuiWindowClass& window_class
	);

	void create_ui (
		const ResourceManager& resource_manager, RenderState& render_state
	);
	void layout_ui (ImGuiID dock_main);

	EditorMode editor_mode = Editing;
	EditorState editor_state;
};

#endif // EDITOR_H
