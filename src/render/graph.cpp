#include "graph.h"

#include "SDL3/SDL_log.h"

RenderGraph::RenderGraph() = default;
RenderGraph::~RenderGraph() = default;

void RenderGraph::add_pass(const RenderPassNode &pass) {
    render_passes.emplace(pass.name, pass);
}

RenderPassNode* RenderGraph::get_render_pass(const std::string& name) {
    RenderPassNode* render_pass = render_passes.contains(name) ? &render_passes[name] : nullptr;
    if (!render_pass) {
        SDL_LogError(SDL_LOG_CATEGORY_RENDER, "Render pass not found.");
        return nullptr;
    }
    return render_pass;
}


void RenderGraph::execute_all(RenderContext& render_context) {
    for (const auto& name : sorted_pass_order) {
        RenderPassNode& render_pass = render_passes[name];
        SDL_Log("Executing pass: %s", name.c_str());
        render_pass.execute(render_context);
    }
}
