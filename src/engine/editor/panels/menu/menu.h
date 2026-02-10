#ifndef MENU_H
#define MENU_H

#include "editor/panels/panel.h"

class Menu final : public IEditorPanel {
  public:
	void draw (EditorContext& editor_context) override;
};

#endif // MENU_H
