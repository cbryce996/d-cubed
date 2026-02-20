#ifndef TEXTURE_H
#define TEXTURE_H

#include <SDL3/SDL.h>
#include <memory>
#include <string>

#include "../../core/storage/state.h"
#include "core/interfaces.h"
#include "core/storage/storage.h"
#include "utils.h"

struct TextureHandle {
	Uint32 id = 0;
};

enum class TextureFormat : uint8_t {
	Invalid,
	RGBA8,
	RGBA16F,
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

struct TextureState final : IState<TextureState> {
	int width;
	int height;
	int num_samplers;
	TextureFormat format;
	TextureUsage usage;

	[[nodiscard]] size_t hash () const override {
		size_t hash = 0;

		hash_combine (hash, std::hash<int> () (width));
		hash_combine (hash, std::hash<int> () (height));
		hash_combine (hash, std::hash<int> () (num_samplers));
		hash_combine (
			hash,
			std::hash<std::underlying_type_t<TextureFormat>>{}(
				static_cast<std::underlying_type_t<TextureFormat>> (format)
			)
		);
		hash_combine (
			hash, std::hash<std::underlying_type_t<TextureUsage>>{}(
					  static_cast<std::underlying_type_t<TextureUsage>> (usage)
				  )
		);

		return hash;
	};

	[[nodiscard]] bool equals (const TextureState& other) const override {
		return width == other.width && height == other.height
			   && num_samplers == other.num_samplers && format == other.format
			   && usage == other.usage;
	};

	[[nodiscard]] std::unique_ptr<TextureState> clone () const override {
		return std::make_unique<TextureState> (*this);
	}
};

struct TextureInstance {
	Handle handle;
	std::string name;
	TextureState state;
};

#endif // TEXTURE_H
