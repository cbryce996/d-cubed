#ifndef TEXTURE_H
#define TEXTURE_H

#include <SDL3/SDL.h>
#include <memory>
#include <string>

#include "core/storage/state.h"
#include "core/storage/storage.h"
#include "utils.h"

enum class TextureUsage : uint32_t;
enum class TextureFormat : uint8_t;

struct TextureHandle {
	uint32_t id = 0;
};

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
