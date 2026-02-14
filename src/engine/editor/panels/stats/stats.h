#ifndef STATS_H
#define STATS_H

#include "editor/panels/panel.h"

class Stats final : public IEditorPanel {
  public:
	void draw (EditorContext& editor_context) override;
};

#endif // STATS_H
