#ifndef PANEL_H
#define PANEL_H

struct EditorContext;

class IEditorPanel {
  public:
	virtual ~IEditorPanel () = default;

	virtual void draw (EditorContext& editor_context);
};

#endif // PANEL_H
