#include "engine.h"

#include "assets/asset.h"
#include "cameras/camera.h"
#include "editor/editor.h"
#include "entity/prefabs/spheres.h"
#include "render/buffers/buffer.h"
#include "render/frame/frame.h"
#include "render/pipelines/sdl/factory.h"
#include "render/render.h"
#include "render/shaders/shader.h"
#include "runtime/clock.h"
#include "runtime/runtime.h"
#include "scene.h"

#include <SDL3/SDL.h>
#include <backends/imgui_impl_sdl3.h>
#include <backends/imgui_impl_sdlgpu3.h>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <imgui.h>

#include <iostream>

Engine::Engine () {
	if (!SDL_Init (SDL_INIT_VIDEO)) {
		std::cerr << "SDL Init failed: " << SDL_GetError () << "\n";
		return;
	}

	SDL_SetLogPriority (SDL_LOG_CATEGORY_RENDER, SDL_LOG_PRIORITY_TRACE);

	window = SDL_CreateWindow (
		"Game", 2560, 1600, SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY
	);

	// SDL_SetWindowFullscreen (window, true);

	gpu_device = SDL_CreateGPUDevice (
		SDL_GPU_SHADERFORMAT_SPIRV | SDL_GPU_SHADERFORMAT_DXIL
			| SDL_GPU_SHADERFORMAT_MSL,
		true, "metal"
	);

	if (!gpu_device) {
		std::cerr << "SDL_CreateGPUDevice failed: " << SDL_GetError () << "\n";
		return;
	}

	if (!SDL_ClaimWindowForGPUDevice (gpu_device, window)) {
		std::cerr << "SDL_ClaimWindowForGPUDevice failed: " << SDL_GetError ()
				  << "\n";
		return;
	}

	std::shared_ptr<ShaderManager> shader_manager
		= std::make_shared<ShaderManager> (gpu_device);
	std::shared_ptr<PipelineManager> pipeline_manager
		= std::make_shared<PipelineManager> (
			std::make_shared<SDLPipelineFactory> (
				gpu_device, window, shader_manager
			)
		);
	std::shared_ptr<BufferManager> buffer_manager
		= std::make_shared<BufferManager> (gpu_device);

	std::shared_ptr<AssetManager> asset_manager
		= std::make_shared<AssetManager> ();
	std::shared_ptr<EditorManager> editor_manager
		= std::make_shared<EditorManager> ();
	std::shared_ptr<FrameManager> frame_manager
		= std::make_shared<FrameManager> ();

	std::shared_ptr<ResourceManager> resource_manager
		= std::make_shared<ResourceManager> (Target{});

	render = std::make_unique<RenderManager> (
		gpu_device, window, shader_manager, pipeline_manager, buffer_manager,
		asset_manager, editor_manager, frame_manager, resource_manager
	);

	runtime = std::make_unique<Runtime> ();
}

Engine::~Engine () {
	SDL_DestroyGPUDevice (gpu_device);
	SDL_DestroyWindow (window);
	SDL_Quit ();
}

void setup_graphite_theme () {
	ImGuiStyle& style = ImGui::GetStyle ();
	ImVec4* colors = style.Colors;

	// --- Graphite palette (neutral, pro) ---
	const ImVec4 bg0 = ImVec4 (0.07f, 0.07f, 0.08f, 1.0f); // window bg
	const ImVec4 bg1 = ImVec4 (0.10f, 0.10f, 0.11f, 1.0f); // frames
	const ImVec4 bg2 = ImVec4 (0.13f, 0.13f, 0.15f, 1.0f); // hovered/child
	const ImVec4 bg3 = ImVec4 (0.17f, 0.17f, 0.20f, 1.0f); // active
	const ImVec4 border = ImVec4 (0.20f, 0.20f, 0.24f, 1.0f);
	const ImVec4 border2 = ImVec4 (0.28f, 0.28f, 0.33f, 1.0f);

	const ImVec4 text = ImVec4 (0.86f, 0.86f, 0.88f, 1.0f);
	const ImVec4 textDim = ImVec4 (0.60f, 0.60f, 0.64f, 1.0f);

	const ImVec4 accent = ImVec4 (0.33f, 0.60f, 0.98f, 1.0f);  // blue
	const ImVec4 accent2 = ImVec4 (0.35f, 0.85f, 0.78f, 1.0f); // teal
	const ImVec4 warn = ImVec4 (0.98f, 0.78f, 0.33f, 1.0f);
	const ImVec4 err = ImVec4 (0.96f, 0.36f, 0.36f, 1.0f);

	// --- Style (professional density) ---
	style.WindowRounding = 6.0f;
	style.ChildRounding = 6.0f;
	style.FrameRounding = 5.0f;
	style.PopupRounding = 6.0f;
	style.ScrollbarRounding = 10.0f;
	style.GrabRounding = 6.0f;
	style.TabRounding = 6.0f;

	style.WindowPadding = ImVec2 (10.0f, 10.0f);
	style.FramePadding = ImVec2 (8.0f, 4.0f);
	style.ItemSpacing = ImVec2 (10.0f, 6.0f);
	style.ItemInnerSpacing = ImVec2 (6.0f, 4.0f);
	style.IndentSpacing = 18.0f;
	style.ScrollbarSize = 14.0f;
	style.GrabMinSize = 10.0f;

	style.WindowBorderSize = 1.0f;
	style.ChildBorderSize = 1.0f;
	style.PopupBorderSize = 1.0f;
	style.FrameBorderSize = 0.0f;
	style.TabBorderSize = 0.0f;

	// --- Colors ---
	colors[ImGuiCol_Text] = text;
	colors[ImGuiCol_TextDisabled] = textDim;

	colors[ImGuiCol_WindowBg] = bg0;
	colors[ImGuiCol_ChildBg] = ImVec4 (bg0.x, bg0.y, bg0.z, 0.0f);
	colors[ImGuiCol_PopupBg] = bg1;

	colors[ImGuiCol_Border] = ImVec4 (border.x, border.y, border.z, 0.85f);
	colors[ImGuiCol_BorderShadow] = ImVec4 (0, 0, 0, 0);

	colors[ImGuiCol_FrameBg] = bg1;
	colors[ImGuiCol_FrameBgHovered] = bg2;
	colors[ImGuiCol_FrameBgActive] = bg3;

	colors[ImGuiCol_TitleBg] = bg1;
	colors[ImGuiCol_TitleBgActive] = bg2;
	colors[ImGuiCol_TitleBgCollapsed] = bg1;

	colors[ImGuiCol_MenuBarBg] = bg1;

	colors[ImGuiCol_ScrollbarBg] = ImVec4 (bg0.x, bg0.y, bg0.z, 0.65f);
	colors[ImGuiCol_ScrollbarGrab] = ImVec4 (bg3.x, bg3.y, bg3.z, 1.0f);
	colors[ImGuiCol_ScrollbarGrabHovered] = border2;
	colors[ImGuiCol_ScrollbarGrabActive] = ImVec4 (
		accent.x, accent.y, accent.z, 0.9f
	);

	colors[ImGuiCol_CheckMark] = accent2;
	colors[ImGuiCol_SliderGrab] = ImVec4 (accent.x, accent.y, accent.z, 0.80f);
	colors[ImGuiCol_SliderGrabActive] = accent;

	colors[ImGuiCol_Button] = bg1;
	colors[ImGuiCol_ButtonHovered] = bg2;
	colors[ImGuiCol_ButtonActive] = bg3;

	colors[ImGuiCol_Header] = bg1;
	colors[ImGuiCol_HeaderHovered] = bg2;
	colors[ImGuiCol_HeaderActive] = bg3;

	colors[ImGuiCol_Separator] = ImVec4 (border.x, border.y, border.z, 0.65f);
	colors[ImGuiCol_SeparatorHovered] = ImVec4 (
		accent.x, accent.y, accent.z, 0.55f
	);
	colors[ImGuiCol_SeparatorActive] = ImVec4 (
		accent.x, accent.y, accent.z, 0.85f
	);

	colors[ImGuiCol_ResizeGrip] = ImVec4 (
		border2.x, border2.y, border2.z, 0.30f
	);
	colors[ImGuiCol_ResizeGripHovered] = ImVec4 (
		accent.x, accent.y, accent.z, 0.45f
	);
	colors[ImGuiCol_ResizeGripActive] = ImVec4 (
		accent.x, accent.y, accent.z, 0.75f
	);

	colors[ImGuiCol_Tab] = bg1;
	colors[ImGuiCol_TabHovered] = bg2;
	colors[ImGuiCol_TabActive] = bg2;
	colors[ImGuiCol_TabUnfocused] = bg1;
	colors[ImGuiCol_TabUnfocusedActive] = bg1;

	colors[ImGuiCol_DockingPreview] = ImVec4 (
		accent.x, accent.y, accent.z, 0.35f
	);
	colors[ImGuiCol_DockingEmptyBg] = bg0;

	colors[ImGuiCol_TableHeaderBg] = bg1;
	colors[ImGuiCol_TableBorderStrong] = ImVec4 (
		border.x, border.y, border.z, 0.75f
	);
	colors[ImGuiCol_TableBorderLight] = ImVec4 (
		border.x, border.y, border.z, 0.35f
	);
	colors[ImGuiCol_TableRowBg] = ImVec4 (0, 0, 0, 0);
	colors[ImGuiCol_TableRowBgAlt] = ImVec4 (1, 1, 1, 0.04f);

	colors[ImGuiCol_TextSelectedBg] = ImVec4 (
		accent.x, accent.y, accent.z, 0.30f
	);
	colors[ImGuiCol_DragDropTarget] = warn;

	colors[ImGuiCol_NavHighlight] = ImVec4 (
		accent.x, accent.y, accent.z, 0.65f
	);
	colors[ImGuiCol_NavWindowingHighlight] = ImVec4 (1, 1, 1, 0.70f);
	colors[ImGuiCol_NavWindowingDimBg] = ImVec4 (0.8f, 0.8f, 0.8f, 0.18f);
	colors[ImGuiCol_ModalWindowDimBg] = ImVec4 (0, 0, 0, 0.45f);

	// Optional: make the plot colors coherent (if you use plots later)
	colors[ImGuiCol_PlotLines] = accent;
	colors[ImGuiCol_PlotLinesHovered] = accent2;
	colors[ImGuiCol_PlotHistogram] = accent2;
	colors[ImGuiCol_PlotHistogramHovered] = accent;

	// Optional: reduce the harshness of hovered text on some items
	colors[ImGuiCol_TextDisabled] = textDim;

	// --- Blue accent (clean, modern) ---
	const ImVec4 blue = ImVec4 (0.33f, 0.60f, 0.98f, 1.0f);		 // primary
	const ImVec4 blueHover = ImVec4 (0.45f, 0.70f, 1.00f, 1.0f); // hover
	const ImVec4 blueSoft = ImVec4 (
		0.33f, 0.60f, 0.98f, 0.35f
	); // translucent fill

	// Selection / highlights
	colors[ImGuiCol_Header] = ImVec4 (blue.x, blue.y, blue.z, 0.18f);
	colors[ImGuiCol_HeaderHovered] = ImVec4 (
		blueHover.x, blueHover.y, blueHover.z, 0.22f
	);
	colors[ImGuiCol_HeaderActive] = ImVec4 (blue.x, blue.y, blue.z, 0.28f);
	colors[ImGuiCol_TextSelectedBg] = ImVec4 (blue.x, blue.y, blue.z, 0.30f);
	colors[ImGuiCol_NavHighlight] = ImVec4 (blue.x, blue.y, blue.z, 0.65f);
	colors[ImGuiCol_DockingPreview] = ImVec4 (blue.x, blue.y, blue.z, 0.30f);

	// Tabs: make them clearly blue when active/hovered
	colors[ImGuiCol_Tab] = ImVec4 (
		0.10f, 0.10f, 0.11f, 1.0f
	); // keep base for inactive
	colors[ImGuiCol_TabUnfocused] = colors[ImGuiCol_Tab];
	colors[ImGuiCol_TabHovered] = ImVec4 (
		blueHover.x, blueHover.y, blueHover.z, 0.35f
	);
	colors[ImGuiCol_TabActive] = ImVec4 (blue.x, blue.y, blue.z, 0.45f);
	colors[ImGuiCol_TabUnfocusedActive] = ImVec4 (
		blue.x, blue.y, blue.z, 0.30f
	);

	// Sliders / checks / grabs: coherent blue accents
	colors[ImGuiCol_CheckMark] = blue;
	colors[ImGuiCol_SliderGrab] = ImVec4 (blue.x, blue.y, blue.z, 0.85f);
	colors[ImGuiCol_SliderGrabActive] = blueHover;
	colors[ImGuiCol_ResizeGripHovered] = ImVec4 (blue.x, blue.y, blue.z, 0.45f);
	colors[ImGuiCol_ResizeGripActive] = ImVec4 (blue.x, blue.y, blue.z, 0.75f);

	// Separators: subtle blue on hover/active
	colors[ImGuiCol_SeparatorHovered] = ImVec4 (blue.x, blue.y, blue.z, 0.55f);
	colors[ImGuiCol_SeparatorActive] = ImVec4 (blue.x, blue.y, blue.z, 0.85f);
}

void Engine::run () {
	running = true;

	runtime->task_scheduler.start ();
	Clock clock (60);

	IMGUI_CHECKVERSION ();
	ImGui::CreateContext ();
	ImGuiIO& io = ImGui::GetIO ();
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

	int ww, wh;
	SDL_GetWindowSize (window, &ww, &wh);
	int dw, dh;
	SDL_GetWindowSizeInPixels (window, &dw, &dh);

	setup_graphite_theme ();

	io.DisplaySize = ImVec2 (static_cast<float> (ww), static_cast<float> (wh));
	io.DisplayFramebufferScale = ImVec2 (
		static_cast<float> (dw) / static_cast<float> (ww),
		static_cast<float> (dh) / static_cast<float> (wh)
	);

	float dpi = io.DisplayFramebufferScale.x;

	ImFontConfig cfg{};
	cfg.SizePixels = 14.0f * dpi;
	// optional knobs:
	cfg.OversampleH = 2;
	cfg.OversampleV = 2;
	cfg.PixelSnapH = true;

	io.Fonts->AddFontDefault (&cfg);

	// Keep “visual size” the same in UI points, but crisp
	io.FontGlobalScale = 0.9f / dpi;

	ImGui_ImplSDL3_InitForSDLGPU (window);

	ImGui_ImplSDLGPU3_InitInfo init_info = {
		gpu_device,
		SDL_GetGPUSwapchainTextureFormat (gpu_device, window),
		SDL_GPU_SAMPLECOUNT_1,
	};

	ImGui_ImplSDLGPU3_Init (&init_info);

	while (running) {
		commit_scene_change ();
		float dt = clock.begin_frame ();

		if (render->editor_manager->editor_state.editor_mode == Editing) {
			SDL_SetWindowRelativeMouseMode (window, false);
		} else {
			SDL_SetWindowRelativeMouseMode (window, true);
		}

		SDL_Event e;
		while (SDL_PollEvent (&e)) {
			ImGui_ImplSDL3_ProcessEvent (&e);
			if (e.type == SDL_EVENT_QUIT)
				running = false;
			if (e.type == SDL_EVENT_KEY_DOWN && !e.key.repeat) {
				if (e.key.key == SDL_GetKeyFromName ("I")) {
					auto& editor = *render->editor_manager;

					editor.editor_state.editor_mode
						= (editor.editor_state.editor_mode == Editing)
							  ? Running
							  : Editing;
				}
			}
		}

		input.poll ();

		const KeyboardInput& keyboard_input = input.get_keyboard_input ();
		MouseInput mouse_input = input.get_mouse_input ();

		if (keyboard_input.keys[SDL_SCANCODE_ESCAPE])
			running = false;
		if (keyboard_input.keys[SDL_SCANCODE_1]) {
			std::unique_ptr<Scene> scene = std::make_unique<Scene> ();

			scene->add_entity (
				std::make_unique<Spheres> (
					glm::vec3 (0, 0, 0), 0.0f, 0.0f, "wave1"
				)
			);

			request_scene (std::move (scene));
		}

		while (clock.should_step_simulation ()) {
			runtime->update (clock.fixed_dt_ms);

			if (active_scene)
				active_scene->update (
					clock.fixed_dt_ms, runtime->simulation_time_ms
				);

			clock.consume_simulation_step ();
		}

		RenderState state{};
		state.scene = active_scene.get ();
		if (active_scene)
			active_scene->collect_drawables (state);

		render->render (
			state, keyboard_input, mouse_input, runtime->simulation_time_ms
		);

		clock.end_frame ();
	}

	runtime->task_scheduler.stop ();
}

void Engine::request_scene (std::unique_ptr<Scene> in_scene) {
	pending_scene = std::move (in_scene);
}

void Engine::commit_scene_change () {
	if (!pending_scene)
		return;

	if (active_scene)
		active_scene->on_unload ();

	active_scene = std::move (pending_scene);

	if (active_scene)
		active_scene->on_load ();
}
