#ifndef CONSOLE_H
#define CONSOLE_H

#include <vector>

#include "editor/panels/panel.h"
#include "imgui.h"

struct ConsoleEntry {
	ImU32 color;
	std::string text;
};

class Console final : public IEditorPanel {
  public:
	void draw (EditorContext& editor_context) override;

  private:
	std::vector<ConsoleEntry> console_entries;
};

#endif // CONSOLE_H
