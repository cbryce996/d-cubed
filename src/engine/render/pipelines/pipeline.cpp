#include "pipeline.h"

PipelineManager::PipelineManager (
	const std::shared_ptr<IPipelineFactory>& factory
)
	: factory (factory) {}

PipelineManager::~PipelineManager () = default;

Pipeline* PipelineManager::get_or_create (const PipelineState& state) {
	const auto it = pipelines.find (state);
	if (it != pipelines.end ()) {
		return &it->second;
	}

	Pipeline* pipeline = factory->create_pipeline (state);

	auto [inserted, _] = pipelines.emplace (state, *pipeline);
	return &inserted->second;
}