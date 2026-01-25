#ifndef CONTEXT_H
#define CONTEXT_H

#include "drawable.h"

#include <vector>
#include <SDL3/SDL.h>

class CameraManager;
class FrameManager;
class ShaderManager;
class BufferManager;
class PipelineManager;

struct RenderContext {
	CameraManager* camera_manager = nullptr;
	PipelineManager* pipeline_manager = nullptr;
	BufferManager* buffer_manager = nullptr;
	ShaderManager* shader_manager = nullptr;
	FrameManager* frame_manager = nullptr;

	std::vector<Drawable>* drawables = nullptr;
	SDL_GPURenderPass* render_pass = nullptr;

	int width = 0;
	int height = 0;

	float time = 0.0f;
};

#endif // CONTEXT_H
