#ifndef THEME_H
#define THEME_H

#include <array>
#include <imgui.h>
#include <string_view>

struct ThemeStyles {
	float WindowRounding = 6.0f;
	float ChildRounding = 6.0f;
	float FrameRounding = 5.0f;
	float PopupRounding = 6.0f;
	float ScrollbarRounding = 10.0f;
	float GrabRounding = 6.0f;
	float TabRounding = 6.0f;

	ImVec2 WindowPadding = ImVec2 (10.0f, 10.0f);
	ImVec2 FramePadding = ImVec2 (8.0f, 4.0f);
	ImVec2 ItemSpacing = ImVec2 (10.0f, 6.0f);
	ImVec2 ItemInnerSpacing = ImVec2 (6.0f, 4.0f);
	float IndentSpacing = 18.0f;
	float ScrollbarSize = 14.0f;
	float GrabMinSize = 10.0f;

	float WindowBorderSize = 1.0f;
	float ChildBorderSize = 1.0f;
	float PopupBorderSize = 1.0f;
	float FrameBorderSize = 0.0f;
	float TabBorderSize = 0.0f;
};

struct Theme {
	std::string_view name{};
	std::array<ImVec4, ImGuiCol_COUNT> colors{};
	ThemeStyles metrics{};
};

namespace Themes {
inline Theme GraphiteBlue () {
	Theme color_scheme;
	color_scheme.name = "Graphite Blue";

	constexpr auto bg0 = ImVec4 (0.07f, 0.07f, 0.08f, 1.0f);
	constexpr auto bg1 = ImVec4 (0.10f, 0.10f, 0.11f, 1.0f);
	constexpr auto bg2 = ImVec4 (0.13f, 0.13f, 0.15f, 1.0f);
	constexpr auto bg3 = ImVec4 (0.17f, 0.17f, 0.20f, 1.0f);
	constexpr auto border = ImVec4 (0.20f, 0.20f, 0.24f, 1.0f);
	constexpr auto border2 = ImVec4 (0.28f, 0.28f, 0.33f, 1.0f);

	constexpr auto text = ImVec4 (0.86f, 0.86f, 0.88f, 1.0f);
	constexpr auto textDim = ImVec4 (0.60f, 0.60f, 0.64f, 1.0f);

	constexpr auto blue = ImVec4 (0.33f, 0.60f, 0.98f, 1.0f);
	constexpr auto blueHover = ImVec4 (0.45f, 0.70f, 1.00f, 1.0f);

	constexpr auto warn = ImVec4 (0.98f, 0.78f, 0.33f, 1.0f);
	constexpr auto err = ImVec4 (0.96f, 0.36f, 0.36f, 1.0f);

	color_scheme.colors.fill (
		ImVec4 (1, 0, 1, 1)
	); // loud magenta if you forget to set something 😉

	// Text
	color_scheme.colors[ImGuiCol_Text] = text;
	color_scheme.colors[ImGuiCol_TextDisabled] = textDim;

	// Backgrounds
	color_scheme.colors[ImGuiCol_WindowBg] = bg0;
	color_scheme.colors[ImGuiCol_ChildBg] = ImVec4 (bg0.x, bg0.y, bg0.z, 0.0f);
	color_scheme.colors[ImGuiCol_PopupBg] = bg1;

	// Borders
	color_scheme.colors[ImGuiCol_Border] = ImVec4 (
		border.x, border.y, border.z, 0.85f
	);
	color_scheme.colors[ImGuiCol_BorderShadow] = ImVec4 (0, 0, 0, 0);

	// Frames
	color_scheme.colors[ImGuiCol_FrameBg] = bg1;
	color_scheme.colors[ImGuiCol_FrameBgHovered] = bg2;
	color_scheme.colors[ImGuiCol_FrameBgActive] = bg3;

	// Title/Menu
	color_scheme.colors[ImGuiCol_TitleBg] = bg1;
	color_scheme.colors[ImGuiCol_TitleBgActive] = bg2;
	color_scheme.colors[ImGuiCol_TitleBgCollapsed] = bg1;
	color_scheme.colors[ImGuiCol_MenuBarBg] = bg1;

	// Scrollbar
	color_scheme.colors[ImGuiCol_ScrollbarBg] = ImVec4 (
		bg0.x, bg0.y, bg0.z, 0.65f
	);
	color_scheme.colors[ImGuiCol_ScrollbarGrab] = ImVec4 (
		bg3.x, bg3.y, bg3.z, 1.0f
	);
	color_scheme.colors[ImGuiCol_ScrollbarGrabHovered] = border2;
	color_scheme.colors[ImGuiCol_ScrollbarGrabActive] = ImVec4 (
		blue.x, blue.y, blue.z, 0.90f
	);

	// Controls
	color_scheme.colors[ImGuiCol_CheckMark] = blue;
	color_scheme.colors[ImGuiCol_SliderGrab] = ImVec4 (
		blue.x, blue.y, blue.z, 0.85f
	);
	color_scheme.colors[ImGuiCol_SliderGrabActive] = blueHover;

	// Buttons
	color_scheme.colors[ImGuiCol_Button] = bg1;
	color_scheme.colors[ImGuiCol_ButtonHovered] = bg2;
	color_scheme.colors[ImGuiCol_ButtonActive] = bg3;

	// Headers (tree, selectable, etc.) – your later override values
	color_scheme.colors[ImGuiCol_Header] = ImVec4 (
		blue.x, blue.y, blue.z, 0.18f
	);
	color_scheme.colors[ImGuiCol_HeaderHovered] = ImVec4 (
		blueHover.x, blueHover.y, blueHover.z, 0.22f
	);
	color_scheme.colors[ImGuiCol_HeaderActive] = ImVec4 (
		blue.x, blue.y, blue.z, 0.28f
	);

	// Separators
	color_scheme.colors[ImGuiCol_Separator] = ImVec4 (
		border.x, border.y, border.z, 0.65f
	);
	color_scheme.colors[ImGuiCol_SeparatorHovered] = ImVec4 (
		blue.x, blue.y, blue.z, 0.55f
	);
	color_scheme.colors[ImGuiCol_SeparatorActive] = ImVec4 (
		blue.x, blue.y, blue.z, 0.85f
	);

	// Resize grips
	color_scheme.colors[ImGuiCol_ResizeGrip] = ImVec4 (
		border2.x, border2.y, border2.z, 0.30f
	);
	color_scheme.colors[ImGuiCol_ResizeGripHovered] = ImVec4 (
		blue.x, blue.y, blue.z, 0.45f
	);
	color_scheme.colors[ImGuiCol_ResizeGripActive] = ImVec4 (
		blue.x, blue.y, blue.z, 0.75f
	);

	// Tabs – your later override values
	color_scheme.colors[ImGuiCol_Tab] = ImVec4 (0.10f, 0.10f, 0.11f, 1.0f);
	color_scheme.colors[ImGuiCol_TabUnfocused]
		= color_scheme.colors[ImGuiCol_Tab];
	color_scheme.colors[ImGuiCol_TabHovered] = ImVec4 (
		blueHover.x, blueHover.y, blueHover.z, 0.35f
	);
	color_scheme.colors[ImGuiCol_TabActive] = ImVec4 (
		blue.x, blue.y, blue.z, 0.45f
	);
	color_scheme.colors[ImGuiCol_TabUnfocusedActive] = ImVec4 (
		blue.x, blue.y, blue.z, 0.30f
	);

	// Docking
	color_scheme.colors[ImGuiCol_DockingPreview] = ImVec4 (
		blue.x, blue.y, blue.z, 0.30f
	);
	color_scheme.colors[ImGuiCol_DockingEmptyBg] = bg0;

	// Tables
	color_scheme.colors[ImGuiCol_TableHeaderBg] = bg1;
	color_scheme.colors[ImGuiCol_TableBorderStrong] = ImVec4 (
		border.x, border.y, border.z, 0.75f
	);
	color_scheme.colors[ImGuiCol_TableBorderLight] = ImVec4 (
		border.x, border.y, border.z, 0.35f
	);
	color_scheme.colors[ImGuiCol_TableRowBg] = ImVec4 (0, 0, 0, 0);
	color_scheme.colors[ImGuiCol_TableRowBgAlt] = ImVec4 (1, 1, 1, 0.04f);

	// Selection + nav
	color_scheme.colors[ImGuiCol_TextSelectedBg] = ImVec4 (
		blue.x, blue.y, blue.z, 0.30f
	);
	color_scheme.colors[ImGuiCol_NavHighlight] = ImVec4 (
		blue.x, blue.y, blue.z, 0.65f
	);
	color_scheme.colors[ImGuiCol_NavWindowingHighlight] = ImVec4 (
		1, 1, 1, 0.70f
	);
	color_scheme.colors[ImGuiCol_NavWindowingDimBg] = ImVec4 (
		0.8f, 0.8f, 0.8f, 0.18f
	);
	color_scheme.colors[ImGuiCol_ModalWindowDimBg] = ImVec4 (0, 0, 0, 0.45f);

	// Plots
	color_scheme.colors[ImGuiCol_PlotLines] = blue;
	color_scheme.colors[ImGuiCol_PlotLinesHovered] = ImVec4 (
		0.35f, 0.85f, 0.78f, 1.0f
	);
	color_scheme.colors[ImGuiCol_PlotHistogram] = ImVec4 (
		0.35f, 0.85f, 0.78f, 1.0f
	);
	color_scheme.colors[ImGuiCol_PlotHistogramHovered] = blue;

	// Misc
	color_scheme.colors[ImGuiCol_DragDropTarget] = warn;

	return color_scheme;
}
}

#endif // THEME_H
