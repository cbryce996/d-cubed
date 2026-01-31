#ifndef EDITOR_H
#define EDITOR_H

#include <imgui.h>

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
};

class EditorManager {
  public:
	void draw_hierarchy (
		const ImGuiWindowClass& window_class, const RenderState& render_state,
		EditorState& editor_state
	);
	void draw_entity_node (IEntity* entity, EditorState& state);
	void draw_inspector (
		const ImGuiWindowClass& window_class, const EditorState& editor_state
	);
	void draw_console (const ImGuiWindowClass& window_class);
	void draw_stats (const ImGuiWindowClass& window_class);

	void draw_main_menu ();
	void draw_viewport (
		const BufferManager& buffer_manager,
		const ImGuiWindowClass& window_class
	);

	void create_ui (const BufferManager& buffer_manager, RenderState& render_state);
	void layout_ui (ImGuiID dock_main);

	EditorState editor_state;
	ViewportState viewport_state;
};

#endif // EDITOR_H
