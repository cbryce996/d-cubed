#include "graph.h"

#include <ranges>
#include <unordered_set>

#include "SDL3/SDL_log.h"

RenderGraph::RenderGraph () = default;
RenderGraph::~RenderGraph () = default;

void RenderGraph::add_pass (const RenderPassInstance& pass) {
	if (render_passes.contains (pass.name)) {
		SDL_LogError (
			SDL_LOG_CATEGORY_RENDER, "Render pass already exists: %s",
			pass.name.c_str ()
		);
		return;
	}

	// transactional insert
	render_passes.emplace (pass.name, pass);

	if (!validate ()) {
		render_passes.erase (pass.name); // rollback
		SDL_LogError (
			SDL_LOG_CATEGORY_RENDER,
			"Invalid render graph state after adding pass: %s",
			pass.name.c_str ()
		);
	}
}
RenderPassInstance* RenderGraph::get_render_pass (const std::string& name) {
	RenderPassInstance* render_pass = render_passes.contains (name)
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
		RenderPassInstance& render_pass = render_passes[name];
		render_pass.execute (render_context, render_pass);
		render_pass.completed = true;
	}
}

bool RenderGraph::validate () {
	sorted_pass_order.clear ();

	std::unordered_set<std::string> visited;
	std::unordered_set<std::string> on_stack;

	for (const auto& pass : render_passes | std::views::values) {
		for (const auto& dep : pass.dependencies) {
			if (!render_passes.contains (dep)) {
				SDL_LogError (
					SDL_LOG_CATEGORY_RENDER, "Missing dependency: %s",
					dep.c_str ()
				);
				return false;
			}
		}
	}

	bool valid = true;

	std::function<void (const std::string&)> dfs =
		[&] (const std::string& node) {
			if (!valid)
				return;

			if (on_stack.contains (node)) {
				SDL_LogError (
					SDL_LOG_CATEGORY_RENDER, "Cycle detected at pass: %s",
					node.c_str ()
				);
				valid = false;
				return;
			}

			if (visited.contains (node))
				return;

			on_stack.insert (node);

			for (const auto& dep : render_passes[node].dependencies) {
				dfs (dep);
			}

			on_stack.erase (node);
			visited.insert (node);
			sorted_pass_order.push_back (node);
		};

	for (const auto& name : render_passes | std::views::keys)
		dfs (name);

	return valid;
}
