#include "inspector.h"

#include "editor/editor.h"
#include "entity/entity.h"
#include "imgui.h"
#include "utils.h"

#include <cstdio>
#include <unordered_map>

void Inspector::draw (EditorContext& editor_context) {
	ImGui::SetNextWindowClass (editor_context.window);
	ImGui::Begin ("Inspector", nullptr, ImGuiWindowFlags_NoCollapse);

	auto& state = editor_context.editor_state;
	IEntity* entity = state.selected_entity;

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

	ImGui::Text ("Transform");
	ImGui::Separator ();

	Transform& transform = entity->transform;
	bool changed = false;

	changed |= ImGui::DragFloat3 (
		"Position", glm::value_ptr (transform.position), 0.05f
	);
	changed |= ImGui::DragFloat3 (
		"Scale", glm::value_ptr (transform.scale), 0.05f
	);
	changed |= ImGui::DragFloat3 (
		"Rotation", glm::value_ptr (transform.rotation), 0.15f
	);

	if (changed) {
		if (IEntity* root = root_of (entity)) {
			root->update_world_transform ();
		}
	}

	ImGui::End ();
}
