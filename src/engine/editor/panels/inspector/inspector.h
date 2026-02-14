#ifndef INSPECTOR_H
#define INSPECTOR_H

#include "editor/panels/panel.h"

class Inspector final : public IEditorPanel {
  public:
	void draw (EditorContext& editor_context) override;
};

#endif // INSPECTOR_H
