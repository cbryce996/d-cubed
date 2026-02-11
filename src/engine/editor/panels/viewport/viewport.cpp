#include "viewport.h"

#include "editor/editor.h"
#include "imgui.h"

void Viewport::draw (EditorContext& editor_context) {
	ImGui::SetNextWindowClass (editor_context.window);

	ImGui::PushStyleVar (ImGuiStyleVar_WindowPadding, ImVec2 (0, 0));
	ImGui::PushStyleVar (ImGuiStyleVar_ItemSpacing, ImVec2 (0, 0));

	const bool open = ImGui::Begin (
		"Viewport", nullptr,
		ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse
			| ImGuiWindowFlags_NoCollapse
	);

	auto end_scope = [&] {
		ImGui::End ();
		ImGui::PopStyleVar (2);
	};

	if (!open) {
		end_scope ();
		return;
	}

	SDL_GPUTexture* tex
		= editor_context.resource_manager.viewport_target.read ();
	if (!tex) {
		ImGui::Text ("Viewport texture not ready...");
		end_scope ();
		return;
	}

	const ImVec2 window_pos = ImGui::GetWindowPos ();
	const ImVec2 content_region_min = ImGui::GetWindowContentRegionMin ();
	const ImVec2 content_region_max = ImGui::GetWindowContentRegionMax ();

	const auto panel_min = ImVec2 (
		window_pos.x + content_region_min.x, window_pos.y + content_region_min.y
	);
	const auto panel_max = ImVec2 (
		window_pos.x + content_region_max.x, window_pos.y + content_region_max.y
	);
	const auto panel_size = ImVec2 (
		panel_max.x - panel_min.x, panel_max.y - panel_min.y
	);

	if (panel_size.x <= 1.0f || panel_size.y <= 1.0f) {
		end_scope ();
		return;
	}

	const float texture_ratio
		= static_cast<float> (
			  editor_context.resource_manager.viewport_target.width
		  )
		  / static_cast<float> (
			  editor_context.resource_manager.viewport_target.height
		  );

	const float panel_ratio = panel_size.x / panel_size.y;

	ImVec2 uv0 (0.0f, 0.0f);
	ImVec2 uv1 (1.0f, 1.0f);

	if (panel_ratio < texture_ratio) {
		const float frac = panel_ratio / texture_ratio;
		const float u0 = (1.0f - frac) * 0.5f;
		uv0.x = u0;
		uv1.x = u0 + frac;
	} else if (panel_ratio > texture_ratio) {
		const float frac = texture_ratio / panel_ratio;
		const float v0 = (1.0f - frac) * 0.5f;
		uv0.y = v0;
		uv1.y = v0 + frac;
	}

	ImDrawList* dl = ImGui::GetWindowDrawList ();
	dl->PushClipRect (panel_min, panel_max, true);

	dl->AddRectFilled (panel_min, panel_max, IM_COL32 (10, 10, 10, 255));

	dl->AddImage (tex, panel_min, panel_max, uv0, uv1);

	{
		constexpr float pad = 10.0f;
		constexpr float box_pad = 6.0f;

		const char* mode_text = (editor_context.editor_state.editor_mode
								 == Editing)
									? "EDITOR MODE"
									: "PLAY MODE";
		const ImU32 mode_col = (editor_context.editor_state.editor_mode
								== Editing)
								   ? IM_COL32 (0, 255, 0, 255)
								   : IM_COL32 (255, 165, 0, 255);

		const ImVec2 text_size = ImGui::CalcTextSize (mode_text);
		const ImVec2 text_pos (panel_min.x + pad, panel_min.y + pad);

		const ImVec2 box_min (text_pos.x - box_pad, text_pos.y - box_pad);
		const ImVec2 box_max (
			text_pos.x + text_size.x + box_pad,
			text_pos.y + text_size.y + box_pad
		);

		dl->AddRectFilled (box_min, box_max, IM_COL32 (30, 30, 30, 80), 4.0f);
		dl->AddText (text_pos, mode_col, mode_text);
	}

	{
		constexpr float pad = 10.0f;
		constexpr float box_pad = 6.0f;

		const ImGuiIO& io = ImGui::GetIO ();
		const ImVec2 fb = io.DisplayFramebufferScale;

		const int tex_w = editor_context.resource_manager.viewport_target.width;
		const int tex_h
			= editor_context.resource_manager.viewport_target.height;

		char buf[256];
		std::snprintf (
			buf, sizeof (buf),
			"Render: %dx%d\nPanel:  %.0fx%.0f\nFB:     %.2fx%.2f", tex_w, tex_h,
			panel_size.x, panel_size.y, fb.x, fb.y
		);

		const ImVec2 text_size = ImGui::CalcTextSize (buf);
		const ImVec2 text_pos (
			panel_max.x - pad - text_size.x, panel_min.y + pad
		);

		const ImVec2 box_min (text_pos.x - box_pad, text_pos.y - box_pad);
		const ImVec2 box_max (
			text_pos.x + text_size.x + box_pad,
			text_pos.y + text_size.y + box_pad
		);

		dl->AddRectFilled (box_min, box_max, IM_COL32 (30, 30, 30, 80), 4.0f);
		dl->AddText (text_pos, IM_COL32 (255, 255, 0, 255), buf);
	}

	dl->PopClipRect ();

	editor_context.editor_state.viewport_hovered = ImGui::IsWindowHovered (
		ImGuiHoveredFlags_AllowWhenBlockedByPopup
	);
	editor_context.editor_state.viewport_focused = ImGui::IsWindowFocused (
		ImGuiFocusedFlags_RootAndChildWindows
	);

	end_scope ();
}
