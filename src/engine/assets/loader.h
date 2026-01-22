#ifndef LOADER_H
#define LOADER_H

#include "../render/block.h"
#include "maths/vector.h"

#include <string>

struct IMeshLoader {
	virtual ~IMeshLoader () = default;
	virtual bool load (
		const std::string& path, std::vector<Vector3>& out_vertices
	) = 0;
};

struct TinyObjMeshLoader : IMeshLoader {
	bool
	load (const std::string& path, std::vector<Vector3>& out_vertices) override;
};

#endif // LOADER_H
