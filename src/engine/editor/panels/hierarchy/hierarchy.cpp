#include "hierarchy.h"

#include "editor/editor.h"
#include "entity/entity.h"
#include "imgui.h"
#include "render/render.h"
#include "scene.h"

#include <ranges>

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
			editor_state.cached_rotation_euler[&entity] = glm::degrees (
				glm::eulerAngles (entity.transform.rotation)
			);
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

	ImGui::PushStyleVar (ImGuiStyleVar_WindowPadding, ImVec2 (8, 6));
	ImGui::PushStyleVar (ImGuiStyleVar_FramePadding, ImVec2 (6, 2));
	ImGui::PushStyleVar (ImGuiStyleVar_ItemSpacing, ImVec2 (6, 2));
	ImGui::PushStyleVar (ImGuiStyleVar_IndentSpacing, 14.0f);

	ImGui::Begin ("Hierarchy", nullptr, ImGuiWindowFlags_NoCollapse);

	ImGui::PushStyleVar (ImGuiStyleVar_ItemSpacing, ImVec2 (6, 1));

	Scene& scene = *editor_context.render_state.scene;

	if (!editor_context.editor_state.selected_entity) {
		for (const auto& entity : scene.scene_entities | std::views::values) {
			if (!entity->parent) {
				editor_context.editor_state.selected_entity = entity.get ();
				break;
			}
		}
	}

	for (const auto& entity : scene.scene_entities | std::views::values) {
		if (!entity->parent) {
			draw_entity_node (*entity.get (), editor_context.editor_state);
		}
	}

	ImGui::PopStyleVar ();

	ImGui::End ();
	ImGui::PopStyleVar (4);
}