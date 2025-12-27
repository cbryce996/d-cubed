#include "world_geometry.h"

#include <boost/polygon/voronoi.hpp>
#include <map>

using namespace boost::polygon;

struct Point {
	int x, y;
	Point(
		int _x,
		int _y
	)
		: x(_x),
		  y(_y) {}
};

namespace boost::polygon {
template <>
struct geometry_concept<Point> {
	typedef point_concept type;
};

template <>
struct point_traits<Point> {
	typedef int coordinate_type;

	static coordinate_type get(
		const Point& p,
		const orientation_2d& orient
	) {
		return (orient == HORIZONTAL) ? p.x : p.y;
	}
};
}  // namespace boost::polygon

std::vector<Region> WorldGeometry::generate_regions(
	const std::vector<Vec2>& points,
	float width,
	float height
) {
	std::vector<Point> pts;
	std::map<const voronoi_diagram<double>::cell_type*, int> region_index;
	std::vector<Region> regions;

	pts.reserve(points.size());
	for (auto [x, y] : points) {
		pts.emplace_back(x, y);
	}

	voronoi_diagram<double> vd;
	construct_voronoi(pts.begin(), pts.end(), &vd);

	regions.resize(points.size());

	// Assign region centers
	for (size_t i = 0; i < points.size(); ++i) {
		regions[i].id = static_cast<int>(i);
		regions[i].center = points[i];
	}

	for (const auto& it : vd.cells()) {
		const auto* cell = &it;
		const auto* edge = cell->incident_edge();
		if (!edge)
			continue;

		const int region_id = cell->source_index();

		std::vector<Vec2> polygon_points;
		std::set<int> neighbor_set;

		const auto* start = edge;
		do {
			if (edge->is_primary()) {
				if (edge->is_finite()) {
					auto v0 = edge->vertex0();
					if (v0) {
						polygon_points.push_back(
							{static_cast<float>(v0->x()),
							 static_cast<float>(v0->y())}
						);
					}
				} else {
					// Handle infinite edges: clip
					// to bounding box and add
					// clipped vertex here
					// (Implement clipping logic
					// here)
				}

				if (edge->twin() && edge->twin()->cell() != cell) {
					int neighbor_id = edge->twin()->cell()->source_index();
					neighbor_set.insert(neighbor_id);
				}
			}

			edge = edge->next();
		} while (edge != start);

		regions[region_id].neighbors.assign(
			neighbor_set.begin(),
			neighbor_set.end()
		);

		if (!polygon_points.empty()) {
			Vec2 centroid{0, 0};
			for (const auto& pt : polygon_points) {
				centroid.x += pt.x;
				centroid.y += pt.y;
			}
			centroid.x /= polygon_points.size();
			centroid.y /= polygon_points.size();

			// Sort points around centroid
			std::sort(
				polygon_points.begin(),
				polygon_points.end(),
				[&](const Vec2& a, const Vec2& b) {
					float angleA =
						std::atan2(a.y - centroid.y, a.x - centroid.x);
					float angleB =
						std::atan2(b.y - centroid.y, b.x - centroid.x);
					return angleA < angleB;
				}
			);

			regions[region_id].polygon = std::move(polygon_points);
			regions[region_id].center = centroid;
		}
	}

	return regions;
}
