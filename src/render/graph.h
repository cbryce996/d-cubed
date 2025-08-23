#ifndef GRAPH_H
#define GRAPH_H

#include <functional>
#include <string>

#include "render.h"
#include "SDL3/SDL_gpu.h"

struct RenderPassNode {
    std::string name;
    std::function<void(RenderContext&)> execute;
    std::vector<std::string> dependencies;
};


class RenderGraph {
public:
    RenderGraph();
    ~RenderGraph();

    void add_pass(const RenderPassNode& pass);
    RenderPassNode* get_render_pass(const std::string& name);
    void execute_all(RenderContext& render_context);

private:
    std::unordered_map<std::string, RenderPassNode> render_passes;
    std::vector<std::string> sorted_pass_order;

    void topological_sort();
};


#endif // GRAPH_H
