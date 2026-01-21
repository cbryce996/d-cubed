#ifndef GRAPH_H
#define GRAPH_H

#include "render/pass.h"
#include <string>

struct RenderContext;

class RenderGraph {
  public:
	RenderGraph ();
	~RenderGraph ();

	void add_pass (const RenderPassInstance& pass);
	RenderPassInstance* get_render_pass (const std::string& name);
	void execute_all (RenderContext& render_context);

  private:
	std::unordered_map<std::string, RenderPassInstance> render_passes;
	std::vector<std::string> sorted_pass_order;

	void topological_sort ();
};

#endif // GRAPH_H
