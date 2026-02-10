#include "hierarchy.h"

#include "editor/editor.h"
#include "entity/entity.h"
#include "imgui.h"
#include "render/render.h"
#include "scene.h"

#include <ranges>

static void refresh_world_from_root (IEntity* entity) {
	if (!entity)
		return;
	if (IEntity* root = root_of (entity)) {
		root->update_world_transform ();
	}
}

void Hierarchy::draw_entity_node (IEntity& entity, EditorState& editor_state) {
	const bool is_leaf = entity.children.empty ();

	ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow
							   | ImGuiTreeNodeFlags_SpanFullWidth
							   | ImGuiTreeNodeFlags_FramePadding;

	if (is_leaf) {
		flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
	}

	ImGui::PushStyleVar (ImGuiStyleVar_IndentSpacing, 8.0f);

	if (editor_state.selected_entity == &entity)
		flags |= ImGuiTreeNodeFlags_Selected;

	bool tree_pushed = false;

	if (is_leaf) {
		ImGui::TreeNodeEx (
			&entity,
			flags | ImGuiTreeNodeFlags_Leaf
				| ImGuiTreeNodeFlags_NoTreePushOnOpen,
			"%s", entity.name.c_str ()
		);
	} else {
		tree_pushed = ImGui::TreeNodeEx (
			&entity, flags, "%s", entity.name.c_str ()
		);
	}

	if (ImGui::IsItemClicked ()) {
		if (editor_state.selected_entity != &entity) {
			editor_state.selected_entity = &entity;

			editor_state.cached_rotation_euler[&entity]
				= entity.transform.rotation;

			refresh_world_from_root (&entity);
		}
	}

	if (tree_pushed) {
		for (auto* child : entity.children) {
			draw_entity_node (*child, editor_state);
		}
		ImGui::TreePop ();
	}

	ImGui::PopStyleVar ();
}

void Hierarchy::draw (EditorContext& editor_context) {
	ImGui::SetNextWindowClass (editor_context.window);
	ImGui::Begin ("Hierarchy", nullptr, ImGuiWindowFlags_NoCollapse);

	Scene& scene = *editor_context.render_state.scene;
	auto& state = editor_context.editor_state;

	if (!state.selected_entity) {
		for (const auto& e : scene.scene_entities | std::views::values) {
			if (!e->parent) {
				state.selected_entity = e.get ();

				state.cached_rotation_euler[state.selected_entity]
					= state.selected_entity->transform.rotation;

				refresh_world_from_root (state.selected_entity);
				break;
			}
		}
	}

	for (const auto& e : scene.scene_entities | std::views::values) {
		if (!e->parent) {
			draw_entity_node (*e.get (), state);
		}
	}

	ImGui::End ();
}
