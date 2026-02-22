#include <cassert>

#include "core/storage/record.h"
#include "core/storage/storage.h"
#include "factory.h"

#include "render/textures/registry.h"
#include "render/textures/texture.h"

uint32_t SDLTextureFactory::bytes_per_pixel (const TextureFormat format) {
	switch (format) {
	case TextureFormat::RGBA8:
		return 4;
	case TextureFormat::RGBA16F:
		return 8;
	case TextureFormat::D32F:
		return 4;
	default:
		return 0;
	}
}

uint64_t SDLTextureFactory::estimate_bytes (const TextureState& state) {
	const uint32_t bytes = bytes_per_pixel (state.format);
	if (bytes == 0)
		return 0;

	const uint64_t width = (state.width > 0)
							   ? static_cast<uint64_t> (state.width)
							   : 0;
	const uint64_t height = (state.height > 0)
								? static_cast<uint64_t> (state.height)
								: 0;

	return width * height * bytes;
}

static SDL_GPUTextureFormat to_sdl_format (const TextureFormat format) {
	switch (format) {
	case TextureFormat::RGBA8:
		return SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM;
	case TextureFormat::RGBA16F:
		return SDL_GPU_TEXTUREFORMAT_R16G16B16A16_FLOAT;
	case TextureFormat::D32F:
		return SDL_GPU_TEXTUREFORMAT_D32_FLOAT;
	default:
		return SDL_GPU_TEXTUREFORMAT_INVALID;
	}
}

static SDL_GPUTextureUsageFlags to_sdl_usage (const TextureUsage usage) {
	SDL_GPUTextureUsageFlags flags = 0;
	if (usage & TextureUsage::ColorTarget)
		flags |= SDL_GPU_TEXTUREUSAGE_COLOR_TARGET;
	if (usage & TextureUsage::DepthStencil)
		flags |= SDL_GPU_TEXTUREUSAGE_DEPTH_STENCIL_TARGET;
	if (usage & TextureUsage::Sampled)
		flags |= SDL_GPU_TEXTUREUSAGE_SAMPLER;
	if (usage & TextureUsage::Storage)
		flags |= SDL_GPU_TEXTUREUSAGE_GRAPHICS_STORAGE_READ;
	return flags;
}

Handle SDLTextureFactory::create (const TextureState& state) {
	SDL_GPUTextureCreateInfo info{};
	info.type = SDL_GPU_TEXTURETYPE_2D;
	info.width = state.width;
	info.height = state.height;
	info.layer_count_or_depth = 1;
	info.num_levels = 1;
	info.sample_count = SDL_GPU_SAMPLECOUNT_1;
	info.format = to_sdl_format (state.format);
	info.usage = to_sdl_usage (state.usage);

	const Handle handle = this->storage.allocate (
		sizeof (TextureRecord), alignof (TextureRecord)
	);
	auto* record = static_cast<TextureRecord*> (this->storage.try_get (handle));
	assert (record);

	record->tex = SDL_CreateGPUTexture (device, &info);
	assert (record->tex);

	record->state = state;
	record->approx_bytes = estimate_bytes (state);
	record->is_target = false;

	if (stats) {
		stats->creates++;
		stats->live_textures++;
		stats->approx_bytes += record->approx_bytes;
		if (stats->approx_bytes > stats->peak_approx_bytes)
			stats->peak_approx_bytes = stats->approx_bytes;
	}

	return handle;
}

void SDLTextureFactory::destroy (Handle handle) {
	auto* record = static_cast<TextureRecord*> (this->storage.try_get (handle));
	if (!record)
		return;

	if (stats) {
		stats->destroys++;
		if (stats->live_textures > 0)
			stats->live_textures--;

		if (record->is_target && stats->live_targets > 0)
			stats->live_targets--;

		if (stats->approx_bytes >= record->approx_bytes)
			stats->approx_bytes -= record->approx_bytes;
		else
			stats->approx_bytes = 0;
	}

	if (record->tex) {
		SDL_ReleaseGPUTexture (device, record->tex);
		record->tex = nullptr;
	}

	this->storage.free (handle);
}