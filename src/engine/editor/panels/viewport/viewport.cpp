#include "viewport.h"

#include <SDL3/SDL_gpu.h>
#include <algorithm>
#include <imgui_internal.h>

#include "editor/editor.h"
#include "imgui.h"
#include "render/textures/registry.h"

static void format_bytes (char* out, size_t out_size, const uint64_t bytes) {
	const auto b = static_cast<double> (bytes);
	const double kb = b / 1024.0;
	const double mb = kb / 1024.0;
	const double gb = mb / 1024.0;

	if (bytes < 1024) {
		std::snprintf (
			out, out_size, "%llu B", static_cast<unsigned long long> (bytes)
		);
	} else if (kb < 1024.0) {
		std::snprintf (out, out_size, "%.1f KB", kb);
	} else if (mb < 1024.0) {
		std::snprintf (out, out_size, "%.2f MB", mb);
	} else {
		std::snprintf (out, out_size, "%.2f GB", gb);
	}
}

static void draw_mode_overlay (
	ImDrawList* draw_list, const ImVec2& panel_min,
	const EditorState& editor_state
) {
	constexpr float padding = 10.0f;
	constexpr float box_padding = 6.0f;

	const char* mode_text = (editor_state.editor_mode == Editing)
								? "EDITOR MODE"
								: "PLAY MODE";

	const ImU32 mode_column = (editor_state.editor_mode == Editing)
								  ? IM_COL32 (0, 255, 0, 255)
								  : IM_COL32 (255, 165, 0, 255);

	const ImVec2 text_size = ImGui::CalcTextSize (mode_text);
	const ImVec2 text_position (panel_min.x + padding, panel_min.y + padding);

	const ImVec2 box_min (
		text_position.x - box_padding, text_position.y - box_padding
	);
	const ImVec2 box_max (
		text_position.x + text_size.x + box_padding,
		text_position.y + text_size.y + box_padding
	);

	draw_list->AddRectFilled (
		box_min, box_max, IM_COL32 (30, 30, 30, 80), 4.0f
	);
	draw_list->AddText (text_position, mode_column, mode_text);
}

static ImRect draw_kv_box_top_right (
	ImDrawList* draw_list, const ImVec2& panel_min, const ImVec2& panel_max,
	const float top_pad, const char* title, const char* const* keys,
	const char* const* vals, const int count, const float right_edge_x = -1.0f
) {
	constexpr float padding = 10.0f;
	constexpr float box_padding = 8.0f;
	constexpr float title_gap = 6.0f;
	constexpr float column_gap = 16.0f;

	constexpr ImU32 background_column = IM_COL32 (30, 30, 30, 100);
	constexpr ImU32 key_column = IM_COL32 (180, 180, 180, 255);
	constexpr ImU32 value_column = IM_COL32 (255, 230, 140, 255);
	constexpr ImU32 title_column = IM_COL32 (200, 220, 255, 255);

	float max_key_width = 0.0f;
	float max_val_width = 0.0f;
	const float line_height = ImGui::GetTextLineHeight ();

	for (int i = 0; i < count; ++i) {
		max_key_width = std::max (
			max_key_width, ImGui::CalcTextSize (keys[i]).x
		);
		max_val_width = std::max (
			max_val_width, ImGui::CalcTextSize (vals[i]).x
		);
	}

	const float title_width = ImGui::CalcTextSize (title).x;
	const float box_width
		= std::max (title_width, max_key_width + column_gap + max_val_width)
		  + box_padding * 2.0f;

	const float box_height = box_padding * 2.0f + line_height + title_gap
							 + (line_height * count);

	const float box_right = (right_edge_x >= 0.0f) ? right_edge_x
												   : (panel_max.x - padding);

	const ImVec2 box_max (box_right, panel_min.y + top_pad + box_height);
	const ImVec2 box_min (box_max.x - box_width, box_max.y - box_height);

	draw_list->AddRectFilled (box_min, box_max, background_column, 4.0f);

	ImVec2 cursor (box_min.x + box_padding, box_min.y + box_padding);
	draw_list->AddText (cursor, title_column, title);
	cursor.y += line_height + title_gap;

	const float key_x = box_min.x + box_padding;
	const float value_right_x = box_max.x - box_padding;

	for (int i = 0; i < count; ++i) {
		draw_list->AddText (ImVec2 (key_x, cursor.y), key_column, keys[i]);

		const ImVec2 value_size = ImGui::CalcTextSize (vals[i]);
		draw_list->AddText (
			ImVec2 (value_right_x - value_size.x, cursor.y), value_column,
			vals[i]
		);

		cursor.y += line_height;
	}

	return ImRect (box_min, box_max);
}

static ImRect draw_viewport_stats_overlay_anchored (
	ImDrawList* draw_list, const ImVec2& panel_min, const ImVec2& panel_max,
	const ImVec2& panel_size, const int texture_width, const int texture_height,
	const float right_edge_x
) {
	const ImGuiIO& io = ImGui::GetIO ();
	const ImVec2 scale = io.DisplayFramebufferScale;

	char value_render[64], value_panel[64], value_scale[64];
	std::snprintf (
		value_render, sizeof (value_render), "%dx%d", texture_width,
		texture_height
	);
	std::snprintf (
		value_panel, sizeof (value_panel), "%.0fx%.0f", panel_size.x,
		panel_size.y
	);
	std::snprintf (
		value_scale, sizeof (value_scale), "%.2fx%.2f", scale.x, scale.y
	);

	const char* keys[] = {"Render", "Panel", "Scale"};
	const char* vals[] = {value_render, value_panel, value_scale};

	return draw_kv_box_top_right (
		draw_list, panel_min, panel_max, 10.0f, "Frame", keys, vals, 3,
		right_edge_x
	);
}

static float bytes_to_mb_u64 (const uint64_t bytes) {
	return static_cast<float> (static_cast<double> (bytes) / (1024.0 * 1024.0));
}

struct RegistryLine {
	const char* key = nullptr;
	const char* val = nullptr;
};

static ImRect draw_texture_registry_debug_box_top_right (
	ImDrawList* draw_list, const ImVec2& panel_min, const ImVec2& panel_max,
	const TextureDebugSnapshot& snap, const float right_edge_x = -1.0f
) {
	constexpr float padding = 10.0f;
	constexpr float box_padding = 8.0f;
	constexpr float title_gap = 6.0f;
	constexpr float section_gap = 4.0f;
	constexpr float column_gap = 16.0f;

	constexpr ImU32 background_column = IM_COL32 (30, 30, 30, 110);
	constexpr ImU32 title_column = IM_COL32 (200, 220, 255, 255);
	constexpr ImU32 section_column = IM_COL32 (180, 220, 255, 255);
	constexpr ImU32 key_column = IM_COL32 (180, 180, 180, 255);
	constexpr ImU32 value_column = IM_COL32 (255, 230, 140, 255);

	char value_viewport_size[64];
	char value_viewport_read_valid[16];
	char value_viewport_write_valid[16];
	char value_viewport_bytes[64];

	std::snprintf (
		value_viewport_size, sizeof (value_viewport_size), "%dx%d",
		snap.viewport_width, snap.viewport_height
	);
	std::snprintf (
		value_viewport_read_valid, sizeof (value_viewport_read_valid), "%s",
		snap.viewport_read_valid ? "YES" : "NO"
	);
	std::snprintf (
		value_viewport_write_valid, sizeof (value_viewport_write_valid), "%s",
		snap.viewport_write_valid ? "YES" : "NO"
	);
	std::snprintf (
		value_viewport_bytes, sizeof (value_viewport_bytes), "%.2f MB",
		bytes_to_mb_u64 (snap.viewport_bytes)
	);

	char value_storage_allocations[32];
	char value_storage_frees[32];
	char value_storage_live[32];
	char value_storage_peak_live[32];
	char value_storage_capacity[32];
	char value_storage_free_list[32];
	char value_storage_bytes_reserved[64];
	char value_storage_bytes_live[64];

	std::snprintf (
		value_storage_allocations, sizeof (value_storage_allocations), "%llu",
		static_cast<unsigned long long> (snap.storage.allocations)
	);
	std::snprintf (
		value_storage_frees, sizeof (value_storage_frees), "%llu",
		static_cast<unsigned long long> (snap.storage.frees)
	);
	std::snprintf (
		value_storage_live, sizeof (value_storage_live), "%u", snap.storage.live
	);
	std::snprintf (
		value_storage_peak_live, sizeof (value_storage_peak_live), "%u",
		snap.storage.peak_live
	);
	std::snprintf (
		value_storage_capacity, sizeof (value_storage_capacity), "%u",
		snap.storage.capacity
	);
	std::snprintf (
		value_storage_free_list, sizeof (value_storage_free_list), "%u",
		snap.storage.free_list
	);
	format_bytes (
		value_storage_bytes_reserved, sizeof (value_storage_bytes_reserved),
		snap.storage.bytes_reserved
	);
	format_bytes (
		value_storage_bytes_live, sizeof (value_storage_bytes_live),
		snap.storage.bytes_live
	);

	char value_pool_acquire_calls[32];
	char value_pool_release_calls[32];
	char value_pool_hits[32];
	char value_pool_misses[32];
	char value_pool_creates[32];
	char value_pool_destroys[32];
	char value_pool_live_buckets[32];
	char value_pool_peak_buckets[32];
	char value_pool_free_handles[32];
	char value_pool_peak_free_handles[32];

	std::snprintf (
		value_pool_acquire_calls, sizeof (value_pool_acquire_calls), "%llu",
		static_cast<unsigned long long> (snap.pool.acquire_calls)
	);
	std::snprintf (
		value_pool_release_calls, sizeof (value_pool_release_calls), "%llu",
		static_cast<unsigned long long> (snap.pool.release_calls)
	);
	std::snprintf (
		value_pool_hits, sizeof (value_pool_hits), "%llu",
		static_cast<unsigned long long> (snap.pool.hits)
	);
	std::snprintf (
		value_pool_misses, sizeof (value_pool_misses), "%llu",
		static_cast<unsigned long long> (snap.pool.misses)
	);
	std::snprintf (
		value_pool_creates, sizeof (value_pool_creates), "%llu",
		static_cast<unsigned long long> (snap.pool.creates)
	);
	std::snprintf (
		value_pool_destroys, sizeof (value_pool_destroys), "%llu",
		static_cast<unsigned long long> (snap.pool.destroys)
	);
	std::snprintf (
		value_pool_live_buckets, sizeof (value_pool_live_buckets), "%u",
		snap.pool.live_buckets
	);
	std::snprintf (
		value_pool_peak_buckets, sizeof (value_pool_peak_buckets), "%u",
		snap.pool.peak_buckets
	);
	std::snprintf (
		value_pool_free_handles, sizeof (value_pool_free_handles), "%llu",
		static_cast<unsigned long long> (snap.pool.free_handles)
	);
	std::snprintf (
		value_pool_peak_free_handles, sizeof (value_pool_peak_free_handles),
		"%llu", static_cast<unsigned long long> (snap.pool.peak_free_handles)
	);

	const RegistryLine lines[] = {
		{"Viewport", nullptr},
		{"Size", value_viewport_size},
		{"Read", value_viewport_read_valid},
		{"Write", value_viewport_write_valid},
		{"Bytes", value_viewport_bytes},

		{"", nullptr}, // spacer (section-ish)
		{"Handle Store (CPU)", nullptr},
		{"Allocs", value_storage_allocations},
		{"Frees", value_storage_frees},
		{"Live", value_storage_live},
		{"Peak", value_storage_peak_live},
		{"Capacity", value_storage_capacity},
		{"FreeList", value_storage_free_list},
		{"Reserved", value_storage_bytes_reserved},
		{"Live Bytes", value_storage_bytes_live},

		{"", nullptr},
		{"Handle Pool (CPU)", nullptr},
		{"Acquire", value_pool_acquire_calls},
		{"Release", value_pool_release_calls},
		{"Hits", value_pool_hits},
		{"Misses", value_pool_misses},
		{"Creates", value_pool_creates},
		{"Destroys", value_pool_destroys},
		{"Buckets", value_pool_live_buckets},
		{"Peak Buckets", value_pool_peak_buckets},
		{"Free Cached", value_pool_free_handles},
		{"Peak Cached", value_pool_peak_free_handles},
	};

	float maximum_key_width = 0.0f;
	float maximum_value_width = 0.0f;
	const float text_line_height = ImGui::GetTextLineHeight ();

	for (const auto& [key, value] : lines) {
		if (!key)
			continue;
		if (value == nullptr)
			continue;

		maximum_key_width = std::max (
			maximum_key_width, ImGui::CalcTextSize (key).x
		);
		maximum_value_width = std::max (
			maximum_value_width, ImGui::CalcTextSize (value).x
		);
	}

	const char* title_text = "Texture Registry";
	const float title_text_width = ImGui::CalcTextSize (title_text).x;

	const float box_width = std::max (
								title_text_width, maximum_key_width + column_gap
													  + maximum_value_width
							)
							+ box_padding * 2.0f;

	float box_height = box_padding * 2.0f + text_line_height + title_gap;
	for (const auto& [key, value] : lines) {
		(void)key;
		box_height += text_line_height;
		if (value == nullptr)
			box_height += section_gap;
	}

	const float box_right_edge_x = (right_edge_x >= 0.0f)
									   ? right_edge_x
									   : (panel_max.x - padding);

	const ImVec2 box_max (box_right_edge_x, panel_min.y + 10.0f + box_height);
	const ImVec2 box_min (box_max.x - box_width, box_max.y - box_height);

	draw_list->AddRectFilled (box_min, box_max, background_column, 4.0f);

	ImVec2 text_cursor (box_min.x + box_padding, box_min.y + box_padding);
	draw_list->AddText (text_cursor, title_column, title_text);
	text_cursor.y += text_line_height + title_gap;

	const float key_column_x = box_min.x + box_padding;
	const float value_right_edge_x = box_max.x - box_padding;

	for (const auto& line : lines) {
		if (!line.key)
			continue;

		const bool is_spacer_line
			= (line.val == nullptr && line.key[0] == '\0');
		if (is_spacer_line) {
			text_cursor.y += section_gap;
			continue;
		}

		const bool is_section_header = (line.val == nullptr);
		if (is_section_header) {
			draw_list->AddText (
				ImVec2 (key_column_x, text_cursor.y), section_column, line.key
			);
			text_cursor.y += text_line_height + section_gap;
			continue;
		}

		draw_list->AddText (
			ImVec2 (key_column_x, text_cursor.y), key_column, line.key
		);

		const ImVec2 value_text_size = ImGui::CalcTextSize (line.val);
		draw_list->AddText (
			ImVec2 (value_right_edge_x - value_text_size.x, text_cursor.y),
			value_column, line.val
		);

		text_cursor.y += text_line_height;
	}

	return ImRect (box_min, box_max);
}

void Viewport::draw (EditorContext& editor_context) {
	ImGui::SetNextWindowClass (editor_context.window);

	ImGui::PushStyleVar (ImGuiStyleVar_WindowPadding, ImVec2 (0, 0));
	ImGui::PushStyleVar (ImGuiStyleVar_ItemSpacing, ImVec2 (0, 0));

	const bool is_window_open = ImGui::Begin (
		"Viewport", nullptr,
		ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse
			| ImGuiWindowFlags_NoCollapse
	);

	auto end_window_scope = [&] {
		ImGui::End ();
		ImGui::PopStyleVar (2);
	};

	if (!is_window_open) {
		end_window_scope ();
		return;
	}

	const Handle viewport_read_texture_handle
		= editor_context.texture_registry.viewport.read ();
	SDL_GPUTexture* viewport_texture
		= editor_context.texture_registry.resolve_texture (
			viewport_read_texture_handle
		);

	if (!viewport_texture) {
		ImGui::Text ("Viewport texture not ready...");
		end_window_scope ();
		return;
	}

	const ImVec2 window_position = ImGui::GetWindowPos ();
	const ImVec2 content_region_min = ImGui::GetWindowContentRegionMin ();
	const ImVec2 content_region_max = ImGui::GetWindowContentRegionMax ();

	const ImVec2 panel_min (
		window_position.x + content_region_min.x,
		window_position.y + content_region_min.y
	);
	const ImVec2 panel_max (
		window_position.x + content_region_max.x,
		window_position.y + content_region_max.y
	);
	const ImVec2 panel_size (
		panel_max.x - panel_min.x, panel_max.y - panel_min.y
	);

	if (panel_size.x <= 1.0f || panel_size.y <= 1.0f) {
		end_window_scope ();
		return;
	}

	const auto viewport_texture_width = static_cast<float> (
		editor_context.texture_registry.viewport.width
	);
	const auto viewport_texture_height = static_cast<float> (
		editor_context.texture_registry.viewport.height
	);

	const float texture_aspect_ratio = viewport_texture_width
									   / viewport_texture_height;
	const float panel_aspect_ratio = panel_size.x / panel_size.y;

	ImVec2 uv0 (0.0f, 0.0f);
	ImVec2 uv1 (1.0f, 1.0f);

	if (panel_aspect_ratio < texture_aspect_ratio) {
		const float visible_fraction = panel_aspect_ratio
									   / texture_aspect_ratio;
		const float left_u = (1.0f - visible_fraction) * 0.5f;
		uv0.x = left_u;
		uv1.x = left_u + visible_fraction;
	} else if (panel_aspect_ratio > texture_aspect_ratio) {
		const float visible_fraction = texture_aspect_ratio
									   / panel_aspect_ratio;
		const float top_v = (1.0f - visible_fraction) * 0.5f;
		uv0.y = top_v;
		uv1.y = top_v + visible_fraction;
	}

	ImDrawList* draw_list = ImGui::GetWindowDrawList ();
	draw_list->PushClipRect (panel_min, panel_max, true);

	draw_list->AddRectFilled (panel_min, panel_max, IM_COL32 (10, 10, 10, 255));
	draw_list->AddImage (viewport_texture, panel_min, panel_max, uv0, uv1);

	draw_mode_overlay (draw_list, panel_min, editor_context.editor_state);

	const int viewport_texture_width_pixels
		= editor_context.texture_registry.viewport.width;
	const int viewport_texture_height_pixels
		= editor_context.texture_registry.viewport.height;

	const TextureDebugSnapshot debug_snapshot
		= editor_context.texture_registry.get_debug_snapshot ();

	const ImRect texture_registry_debug_box
		= draw_texture_registry_debug_box_top_right (
			draw_list, panel_min, panel_max, debug_snapshot
		);

	constexpr float overlay_gap = 10.0f;
	const float frame_right_edge_x = texture_registry_debug_box.Min.x
									 - overlay_gap;

	draw_viewport_stats_overlay_anchored (
		draw_list, panel_min, panel_max, panel_size,
		viewport_texture_width_pixels, viewport_texture_height_pixels,
		frame_right_edge_x
	);

	draw_list->PopClipRect ();

	editor_context.editor_state.viewport_hovered = ImGui::IsWindowHovered (
		ImGuiHoveredFlags_AllowWhenBlockedByPopup
	);

	editor_context.editor_state.viewport_focused = ImGui::IsWindowFocused (
		ImGuiFocusedFlags_RootAndChildWindows
	);

	end_window_scope ();
}