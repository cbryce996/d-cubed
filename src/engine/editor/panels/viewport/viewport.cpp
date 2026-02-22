#include "viewport.h"

#include <SDL3/SDL_gpu.h>
#include <algorithm>
#include <cstdio>
#include <imgui_internal.h>

#include "editor/editor.h"
#include "imgui.h"
#include "render/textures/registry.h"

static void draw_mode_overlay (
	ImDrawList* dl, const ImVec2& panel_min, const EditorState& editor_state
) {
	constexpr float pad = 10.0f;
	constexpr float box_pad = 6.0f;

	const char* mode_text = (editor_state.editor_mode == Editing)
								? "EDITOR MODE"
								: "PLAY MODE";
	const ImU32 mode_col = (editor_state.editor_mode == Editing)
							   ? IM_COL32 (0, 255, 0, 255)
							   : IM_COL32 (255, 165, 0, 255);

	const ImVec2 text_size = ImGui::CalcTextSize (mode_text);
	const ImVec2 text_pos (panel_min.x + pad, panel_min.y + pad);

	const ImVec2 box_min (text_pos.x - box_pad, text_pos.y - box_pad);
	const ImVec2 box_max (
		text_pos.x + text_size.x + box_pad, text_pos.y + text_size.y + box_pad
	);

	dl->AddRectFilled (box_min, box_max, IM_COL32 (30, 30, 30, 80), 4.0f);
	dl->AddText (text_pos, mode_col, mode_text);
}

// Returns the box rect so the caller can stack another box beneath it OR place
// another beside it.
static ImRect draw_kv_box_top_right (
	ImDrawList* dl, const ImVec2& panel_min, const ImVec2& panel_max,
	float top_pad, const char* title, const char* const* keys,
	const char* const* vals, int count,
	float right_edge_x
	= -1.0f // NEW: anchor box_max.x here (defaults to panel_max.x - pad)
) {
	constexpr float pad = 10.0f;
	constexpr float box_pad = 8.0f;
	constexpr float title_gap = 6.0f;
	constexpr float col_gap = 16.0f;

	const ImU32 bg_col = IM_COL32 (30, 30, 30, 100);
	const ImU32 key_col = IM_COL32 (180, 180, 180, 255);
	const ImU32 val_col = IM_COL32 (
		255, 230, 140, 255
	); // warm highlight for values
	const ImU32 title_col = IM_COL32 (200, 220, 255, 255);

	// Measure widths
	float max_key_w = 0.0f;
	float max_val_w = 0.0f;
	float line_h = ImGui::GetTextLineHeight ();

	for (int i = 0; i < count; ++i) {
		max_key_w = std::max (max_key_w, ImGui::CalcTextSize (keys[i]).x);
		max_val_w = std::max (max_val_w, ImGui::CalcTextSize (vals[i]).x);
	}

	const float title_w = ImGui::CalcTextSize (title).x;
	const float box_w = std::max (title_w, max_key_w + col_gap + max_val_w)
						+ box_pad * 2.0f;
	const float box_h = box_pad * 2.0f + line_h /*title*/ + title_gap
						+ (line_h * count);

	// Top-right placement (or custom right-edge)
	const float box_right = (right_edge_x >= 0.0f) ? right_edge_x
												   : (panel_max.x - pad);

	const ImVec2 box_max (box_right, panel_min.y + top_pad + box_h);
	const ImVec2 box_min (box_max.x - box_w, box_max.y - box_h);

	dl->AddRectFilled (box_min, box_max, bg_col, 4.0f);

	// Title
	ImVec2 cursor (box_min.x + box_pad, box_min.y + box_pad);
	dl->AddText (cursor, title_col, title);
	cursor.y += line_h + title_gap;

	// Columns
	const float key_x = box_min.x + box_pad;
	const float val_right_x = box_max.x - box_pad; // values right-anchored here

	for (int i = 0; i < count; ++i) {
		dl->AddText (ImVec2 (key_x, cursor.y), key_col, keys[i]);

		const ImVec2 val_size = ImGui::CalcTextSize (vals[i]);
		dl->AddText (
			ImVec2 (val_right_x - val_size.x, cursor.y), val_col, vals[i]
		);

		cursor.y += line_h;
	}

	return ImRect (box_min, box_max);
}

static ImRect draw_texture_pool_overlay_top_right (
	ImDrawList* dl, const ImVec2& panel_min, const ImVec2& panel_max,
	const TextureRegistryStats& s
) {
	char v_bytes[64], v_textures[32], v_created[32], v_destroyed[32];
	char v_reuses[32], v_misses[32], v_resizes[32];

	const double mb = static_cast<double> (s.approx_bytes) / (1024.0 * 1024.0);
	std::snprintf (v_bytes, sizeof (v_bytes), "%.2f MB", mb);
	std::snprintf (v_textures, sizeof (v_textures), "%u", s.live_textures);
	std::snprintf (v_created, sizeof (v_created), "%u", s.creates);
	std::snprintf (v_destroyed, sizeof (v_destroyed), "%u", s.destroys);
	std::snprintf (v_reuses, sizeof (v_reuses), "%u", s.reuses);
	std::snprintf (v_misses, sizeof (v_misses), "%u", s.misses);
	std::snprintf (v_resizes, sizeof (v_resizes), "%u", s.resizes);

	const char* keys[] = {"Bytes",	"Textures", "Created", "Destroyed",
						  "Reuses", "Misses",	"Resizes"};

	const char* vals[] = {v_bytes,	v_textures, v_created, v_destroyed,
						  v_reuses, v_misses,	v_resizes};

	return draw_kv_box_top_right (
		dl, panel_min, panel_max, 10.0f, "Textures", keys, vals,
		std::size (keys)
	);
}

static ImRect draw_viewport_stats_overlay_anchored (
	ImDrawList* dl, const ImVec2& panel_min, const ImVec2& panel_max,
	const ImVec2& panel_size, int tex_w, int tex_h, float right_edge_x
) {
	const ImGuiIO& io = ImGui::GetIO ();
	const ImVec2 fb = io.DisplayFramebufferScale;

	char v_render[64], v_panel[64], v_fb[64];
	std::snprintf (v_render, sizeof (v_render), "%dx%d", tex_w, tex_h);
	std::snprintf (
		v_panel, sizeof (v_panel), "%.0fx%.0f", panel_size.x, panel_size.y
	);
	std::snprintf (v_fb, sizeof (v_fb), "%.2fx%.2f", fb.x, fb.y);

	const char* keys[] = {"Render", "Panel", "FB Scale"};
	const char* vals[] = {v_render, v_panel, v_fb};

	return draw_kv_box_top_right (
		dl, panel_min, panel_max, 10.0f, "Frame", keys, vals, 3, right_edge_x
	);
}

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

	const Handle viewport_read_handle
		= editor_context.texture_registry.viewport.read ();
	SDL_GPUTexture* viewport_texture
		= editor_context.texture_registry.resolve_texture (
			viewport_read_handle
		);

	if (!viewport_texture) {
		ImGui::Text ("Viewport texture not ready...");
		end_scope ();
		return;
	}

	const ImVec2 window_pos = ImGui::GetWindowPos ();
	const ImVec2 content_region_min = ImGui::GetWindowContentRegionMin ();
	const ImVec2 content_region_max = ImGui::GetWindowContentRegionMax ();

	const ImVec2 panel_min (
		window_pos.x + content_region_min.x, window_pos.y + content_region_min.y
	);
	const ImVec2 panel_max (
		window_pos.x + content_region_max.x, window_pos.y + content_region_max.y
	);
	const ImVec2 panel_size (
		panel_max.x - panel_min.x, panel_max.y - panel_min.y
	);

	if (panel_size.x <= 1.0f || panel_size.y <= 1.0f) {
		end_scope ();
		return;
	}

	const float tex_w = static_cast<float> (
		editor_context.texture_registry.viewport.width
	);
	const float tex_h = static_cast<float> (
		editor_context.texture_registry.viewport.height
	);
	const float texture_ratio = tex_w / tex_h;
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
	dl->AddImage (viewport_texture, panel_min, panel_max, uv0, uv1);

	// Overlays
	draw_mode_overlay (dl, panel_min, editor_context.editor_state);

	const int i_tex_w = editor_context.texture_registry.viewport.width;
	const int i_tex_h = editor_context.texture_registry.viewport.height;

	// Pool on the far right
	const ImRect pool_box = draw_texture_pool_overlay_top_right (
		dl, panel_min, panel_max,
		editor_context.texture_registry.get_pool_stats ()
	);

	// Stats to the LEFT of pool
	constexpr float gap = 10.0f;
	const float stats_right_edge = pool_box.Min.x - gap;

	draw_viewport_stats_overlay_anchored (
		dl, panel_min, panel_max, panel_size, i_tex_w, i_tex_h, stats_right_edge
	);

	dl->PopClipRect ();

	editor_context.editor_state.viewport_hovered = ImGui::IsWindowHovered (
		ImGuiHoveredFlags_AllowWhenBlockedByPopup
	);
	editor_context.editor_state.viewport_focused = ImGui::IsWindowFocused (
		ImGuiFocusedFlags_RootAndChildWindows
	);

	end_scope ();
}