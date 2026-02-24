#ifndef TYPES_H
#define TYPES_H
#include <cstdint>

enum class CompareOp : uint8_t {
	Never,
	Less,
	Equal,
	LessOrEqual,
	Greater,
	NotEqual,
	GreaterOrEqual,
	Always
};

enum class LoadOp : uint8_t { Load, Clear, DontCare };

enum class StoreOp : uint8_t { Store, DontCare };

struct Color4 {
	float r = 0.0f;
	float g = 0.0f;
	float b = 0.0f;
	float a = 1.0f;
};

enum class PrimitiveType : uint8_t {
	TriangleList,
	TriangleStrip,
	LineList,
	LineStrip,
	PointList
};

enum class CullMode : uint8_t { None, Front, Back };

enum class TextureFormat : uint8_t {
	Invalid,
	RGBA8,
	RGBA16F,
	BGRA8,
	D32F,
};

enum class TextureUsage : uint32_t {
	None = 0,
	ColorTarget = 1u << 0,
	DepthStencil = 1u << 1,
	Sampled = 1u << 2,
	Storage = 1u << 3,
};

inline TextureUsage operator| (TextureUsage a, TextureUsage b) {
	return static_cast<TextureUsage> (
		static_cast<uint32_t> (a) | static_cast<uint32_t> (b)
	);
}
inline bool operator& (TextureUsage a, TextureUsage b) {
	return (static_cast<uint32_t> (a) & static_cast<uint32_t> (b)) != 0;
}

#endif // TYPES_H
