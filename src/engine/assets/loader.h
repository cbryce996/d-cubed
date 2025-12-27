#ifndef LOADER_H
#define LOADER_H
#include <string>

#include "mesh.h"

struct IMeshLoader {
	virtual ~IMeshLoader () = default;
	virtual bool load (
		const std::string& path, std::vector<Vertex>& out_vertices
	) = 0;
};

struct TinyObjMeshLoader : IMeshLoader {
	bool
	load (const std::string& path, std::vector<Vertex>& out_vertices) override;
};

#endif // LOADER_H
