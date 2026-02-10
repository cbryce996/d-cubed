#include "inspector.h"

#include "editor/editor.h"
#include "entity/entity.h"
#include "imgui.h"
#include "render/render.h"
#include "scene.h"

void Inspector::draw (EditorContext& editor_context) {
	ImGui::SetNextWindowClass (editor_context.window);
	ImGui::Begin ("Inspector", nullptr, ImGuiWindowFlags_NoCollapse);

	IEntity* entity = editor_context.editor_state.selected_entity;

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

	ImGui::DragFloat3 ("Position", glm::value_ptr (transform.position), 0.05f);
	ImGui::DragFloat3 ("Scale", glm::value_ptr (transform.scale), 0.05f);

	glm::vec3& euler
		= editor_context.editor_state.cached_rotation_euler[entity];

	if (ImGui::DragFloat3 ("Rotation", glm::value_ptr (euler), 0.05f)) {
		transform.rotation = glm::quat (glm::radians (euler));
	}

	ImGui::End ();
}