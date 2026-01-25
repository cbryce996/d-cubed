#include "pipeline.h"
#include "render/render.h"

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