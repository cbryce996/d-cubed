#include "pipeline.h"

#include "render/block.h"
#include "render/buffers/buffer.h"
#include "render/render.h"
#include "utils.h"

PipelineManager::PipelineManager (
	const std::shared_ptr<IPipelineFactory>& factory
)
	: factory (factory) {}

PipelineManager::~PipelineManager () = default;

bool PipelineState::operator== (const PipelineState& other) const {
	return render_pass_state == other.render_pass_state
		   && material_state == other.material_state;
}

Pipeline* PipelineManager::get_or_create (const PipelineState& state) {
	const auto it = pipelines.find (state);
	if (it != pipelines.end ()) {
		return &it->second;
	}

	Pipeline* pipeline = factory->create_pipeline (state);

	auto [inserted, _] = pipelines.emplace (state, *pipeline);
	return &inserted->second;
}

void PipelineManager::push_uniforms (
	const std::vector<UniformBinding>& uniform_bindings,
	const BufferManager& buffer_manager
) const {
	assert (!uniform_bindings.empty ());

	for (const auto& [name, slot, data, size, stage] : uniform_bindings) {
		assert (data);
		assert (size % UNIFORM_ALIGNMENT == 0);
		assert (size > 0);

		if (stage == ShaderStage::Vertex || stage == ShaderStage::Both) {
			SDL_PushGPUVertexUniformData (
				buffer_manager.command_buffer, slot, data, size
			);
		}

		if (stage == ShaderStage::Fragment || stage == ShaderStage::Both) {
			SDL_PushGPUFragmentUniformData (
				buffer_manager.command_buffer, slot, data, size
			);
		}
	}
}