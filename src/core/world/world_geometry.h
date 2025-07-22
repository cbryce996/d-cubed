#ifndef WORLD_GEOMETRY_H
#define WORLD_GEOMETRY_H

#include <boost/polygon/voronoi.hpp>
#include <vector>

struct Vec2 {
	float x, y;

	[[nodiscard]] float distance_squared(const Vec2& other) const {
		const float dx = x - other.x;
		const float dy = y - other.y;
		return dx * dx + dy * dy;
	}
};

inline bool operator<(
	const Vec2& a,
	const Vec2& b
) {
	return a.y < b.y || (a.y == b.y && a.x < b.x);
}

inline Vec2 operator+(
	const Vec2& a,
	const Vec2& b
) {
	return {a.x + b.x, a.y + b.y};
}

inline Vec2 operator/(
	const Vec2& a,
	float val
) {
	return {a.x / val, a.y / val};
}

inline bool operator==(
	const Vec2& a,
	const Vec2& b
) {
	return a.x == b.x && a.y == b.y;
}

struct Region {
	int id;
	Vec2 center;
	std::vector<Vec2> polygon;
	std::vector<int> neighbors;
};

class WorldGeometry {
   public:
	static std::vector<Region> generate_regions(
		const std::vector<Vec2>& points,
		float width,
		float height
	);
};

#endif
