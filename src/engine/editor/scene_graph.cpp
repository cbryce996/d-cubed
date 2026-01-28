#include "scene_graph.h"

#include "entity/entity.h"

#include <imgui.h>
#include <ranges>

static void draw_entity_node (IEntity* entity, EditorState& state) {
	ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow
							   | ImGuiTreeNodeFlags_SpanAvailWidth;

	if (state.selected_entity == entity)
		flags |= ImGuiTreeNodeFlags_Selected;

	const bool opened = ImGui::TreeNodeEx (
		(void*)entity, flags, "%s", entity->name.c_str ()
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

namespace EditorPanels {

void draw_scene_graph (Scene& scene, EditorState& state) {
	if (!state.show_scene_graph)
		return;

	ImGui::Begin ("Scene Graph");

	for (auto& entity : scene.scene_entities | std::views::values) {
		if (!entity->parent) {
			draw_entity_node (entity.get (), state);
		}
	}

	ImGui::End ();
}

}
