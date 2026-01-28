#ifndef EDITOR_H
#define EDITOR_H

class IEntity;

struct EditorState {
	IEntity* selected_entity = nullptr;
	bool show_scene_graph = true;
};

#endif // EDITOR_H
