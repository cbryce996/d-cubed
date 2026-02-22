#ifndef EDITOR_H
#define EDITOR_H

#include <glm/glm.hpp>
#include <imgui.h>
#include <unordered_map>

#include "panels/panel.h"

class TextureRegistry;
struct Handle;
class TextureTarget;
struct RenderState;
class BufferManager;
class IEntity;

enum EditorMode { Editing, Running };

struct EditorState {
	EditorMode editor_mode = Editing;

	IEntity* selected_entity = nullptr;
	std::unordered_map<IEntity*, glm::vec3> cached_rotation_euler;

	// TODO: Move out of here
	bool viewport_hovered = false;
	bool viewport_focused = false;
	bool viewport_show_overlay = true;
	bool viewport_show_stats = true;
	bool viewport_show_toolbar = true;
};

struct EditorContext {
	EditorState& editor_state;
	RenderState& render_state;
	TextureRegistry& texture_registry;
	ImGuiWindowClass* window = nullptr;
};

class EditorManager {
  public:
	EditorManager ();

	void
	create_ui (TextureRegistry& texture_registry, RenderState& render_state);
	void layout_ui (ImGuiID dock_base);

	std::vector<std::unique_ptr<IEditorPanel>> panels;
	EditorState editor_state;
};

#endif // EDITOR_H
