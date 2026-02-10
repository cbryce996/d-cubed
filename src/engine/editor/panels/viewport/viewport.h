#ifndef VIEWPORT_H
#define VIEWPORT_H

#include "editor/panels/panel.h"

class Viewport final : public IEditorPanel {
  public:
	void draw (EditorContext& editor_context) override;
};

#endif // VIEWPORT_H
