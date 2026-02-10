#ifndef HIERARCHY_H
#define HIERARCHY_H

#include "editor/panels/panel.h"

struct EditorState;
class IEntity;

class Hierarchy final : public IEditorPanel {
  public:
	void draw (EditorContext& editor_context) override;
	void draw_entity_node (IEntity& entity, EditorState& editor_state);
};

#endif // HIERARCHY_H
