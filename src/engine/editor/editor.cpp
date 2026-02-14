#include "editor.h"

#include "render/buffers/buffer.h"
#include "render/resources/resources.h"

#include <imgui.h>
#include <imgui_internal.h>

#include "panels/console/console.h"
#include "panels/hierarchy/hierarchy.h"
#include "panels/inspector/inspector.h"
#include "panels/menu/menu.h"
#include "panels/stats/stats.h"
#include "panels/viewport/viewport.h"
#include "themes/theme.h"

EditorManager::EditorManager () {
	panels.reserve (6);

	panels.emplace_back (std::make_unique<Viewport> ());
	panels.emplace_back (std::make_unique<Hierarchy> ());
	panels.emplace_back (std::make_unique<Inspector> ());
	panels.emplace_back (std::make_unique<Stats> ());
	panels.emplace_back (std::make_unique<Console> ());
	panels.emplace_back (std::make_unique<Menu> ());
}

void EditorManager::apply_theme (const Theme& theme) {
	ImGuiStyle& style = ImGui::GetStyle ();

	ImGui::StyleColorsDark (&style);

	style.WindowRounding = theme.metrics.WindowRounding;
	style.ChildRounding = theme.metrics.ChildRounding;
	style.FrameRounding = theme.metrics.FrameRounding;
	style.PopupRounding = theme.metrics.PopupRounding;
	style.ScrollbarRounding = theme.metrics.ScrollbarRounding;
	style.GrabRounding = theme.metrics.GrabRounding;
	style.TabRounding = theme.metrics.TabRounding;

	style.WindowPadding = theme.metrics.WindowPadding;
	style.FramePadding = theme.metrics.FramePadding;
	style.ItemSpacing = theme.metrics.ItemSpacing;
	style.ItemInnerSpacing = theme.metrics.ItemInnerSpacing;
	style.IndentSpacing = theme.metrics.IndentSpacing;
	style.ScrollbarSize = theme.metrics.ScrollbarSize;
	style.GrabMinSize = theme.metrics.GrabMinSize;

	style.WindowBorderSize = theme.metrics.WindowBorderSize;
	style.ChildBorderSize = theme.metrics.ChildBorderSize;
	style.PopupBorderSize = theme.metrics.PopupBorderSize;
	style.FrameBorderSize = theme.metrics.FrameBorderSize;
	style.TabBorderSize = theme.metrics.TabBorderSize;

	for (int i = 0; i < ImGuiCol_COUNT; ++i) {
		style.Colors[i] = theme.colors[i];
	}
}

void EditorManager::create_ui (
	ResourceManager& resource_manager, RenderState& render_state
) {
	static bool theme_applied = false;
	if (!theme_applied) {
		apply_theme (Themes::GraphiteBlue ());
		theme_applied = true;
	}

	constexpr ImGuiDockNodeFlags dock_flags
		= ImGuiDockNodeFlags_PassthruCentralNode;

	const ImGuiID dock_space_id = ImGui::DockSpaceOverViewport (
		ImGui::GetID ("MainDockspace"), ImGui::GetMainViewport (), dock_flags,
		nullptr
	);

	static bool built = false;
	if (!built) {
		layout_ui (dock_space_id);
		built = true;
	}

	ImGuiWindowClass docked_panel_class;
	docked_panel_class.DockNodeFlagsOverrideSet
		= ImGuiDockNodeFlags_NoWindowMenuButton;

	EditorContext editor_context{
		.window = &docked_panel_class,
		.resource_manager = resource_manager,
		.render_state = render_state,
		.editor_state = editor_state,
	};

	for (const auto& panel : panels) {
		panel->draw (editor_context);
	}
}

void EditorManager::layout_ui (ImGuiID dock_base) {
	ImGui::DockBuilderRemoveNode (dock_base);
	ImGui::DockBuilderAddNode (dock_base, ImGuiDockNodeFlags_DockSpace);
	ImGui::DockBuilderSetNodeSize (
		dock_base, ImGui::GetMainViewport ()->WorkSize
	);

	ImGuiID dock_left = ImGui::DockBuilderSplitNode (
		dock_base, ImGuiDir_Left, 0.20f, nullptr, &dock_base
	);
	ImGuiID dock_right = ImGui::DockBuilderSplitNode (
		dock_base, ImGuiDir_Right, 0.25f, nullptr, &dock_base
	);
	ImGuiID dock_bottom = ImGui::DockBuilderSplitNode (
		dock_base, ImGuiDir_Down, 0.25f, nullptr, &dock_base
	);

	ImGuiID dock_stats = ImGui::DockBuilderSplitNode (
		dock_bottom, ImGuiDir_Right, 0.35f, nullptr, &dock_bottom
	);

	ImGui::DockBuilderDockWindow ("Viewport", dock_base);
	ImGui::DockBuilderDockWindow ("Hierarchy", dock_left);
	ImGui::DockBuilderDockWindow ("Inspector", dock_right);
	ImGui::DockBuilderDockWindow ("Console", dock_bottom);
	ImGui::DockBuilderDockWindow ("Stats", dock_stats);

	ImGui::DockBuilderFinish (dock_base);
}
