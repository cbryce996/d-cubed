#include "graph.h"

#include <ranges>
#include <unordered_set>

#include "SDL3/SDL_log.h"

RenderGraph::RenderGraph () = default;
RenderGraph::~RenderGraph () = default;

void RenderGraph::add_pass (const RenderPassNode& pass) {
	render_passes.emplace (pass.name, pass);
	topological_sort ();
}

RenderPassNode* RenderGraph::get_render_pass (const std::string& name) {
	RenderPassNode* render_pass = render_passes.contains (name)
									  ? &render_passes[name]
									  : nullptr;
	if (!render_pass) {
		SDL_LogError (SDL_LOG_CATEGORY_RENDER, "Render pass not found.");
		return nullptr;
	}
	return render_pass;
}

void RenderGraph::execute_all (RenderContext& render_context) {
	for (const std::string& name : sorted_pass_order) {
		RenderPassNode& render_pass = render_passes[name];
		render_pass.execute (render_context);
	}
}

void RenderGraph::topological_sort () {
	sorted_pass_order.clear ();

	std::unordered_set<std::string> visited;
	std::unordered_set<std::string> on_stack;

	std::function<void (const std::string&)> visit =
		[&] (const std::string& name) {
			if (on_stack.contains (name)) {
				SDL_LogError (
					SDL_LOG_CATEGORY_RENDER,
					"Cycle detected in render graph at pass: %s", name.c_str ()
				);
				throw std::runtime_error ("Cycle in render graph");
			}

			if (visited.contains (name))
				return;

			auto it = render_passes.find (name);
			if (it == render_passes.end ()) {
				SDL_LogError (
					SDL_LOG_CATEGORY_RENDER,
					"Unknown render pass dependency: %s", name.c_str ()
				);
				throw std::runtime_error ("Missing dependency in render graph");
			}

			on_stack.insert (name);

			for (const std::string& dep : it->second.dependencies)
				visit (dep);

			on_stack.erase (name);
			visited.insert (name);
			sorted_pass_order.push_back (name);
		};

	for (const auto& name : render_passes | std::views::keys)
		visit (name);
}
