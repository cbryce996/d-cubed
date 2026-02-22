#ifndef TEXTURE_REGISTRY_H
#define TEXTURE_REGISTRY_H

#include "core/storage/maps/slot.h"
#include "core/storage/policies/pool.h"
#include "core/storage/record.h"
#include "sdl/factory/factory.h"
#include "sdl/target.h"
#include "texture.h"

struct TextureRegistryStats {
	uint32_t live_textures = 0;
	uint32_t live_targets = 0;
	uint32_t live_samplers = 0;

	uint64_t approx_bytes = 0;
	uint64_t peak_approx_bytes = 0;

	uint32_t creates = 0;
	uint32_t destroys = 0;
	uint32_t reuses = 0;
	uint32_t misses = 0;
	uint32_t resizes = 0;
};

class TextureRegistry {
  public:
	explicit TextureRegistry (SDL_GPUDevice* device)
		: viewport (storage), texture_factory (device, storage, &stats),
		  texture_pool (texture_factory) {}

	TextureTarget viewport;

	void ensure_viewport_target (const int new_width, const int new_height) {
		const int width = (new_width > 0) ? new_width : 2;
		const int height = (new_height > 0) ? new_height : 2;

		TextureState state{};
		state.width = width;
		state.height = height;
		state.num_samplers = 1;
		state.format = TextureFormat::RGBA8;
		state.usage = TextureUsage::ColorTarget | TextureUsage::Sampled;

		const bool had_read = storage.valid (viewport.read ());
		const bool had_write = storage.valid (viewport.write ());
		const bool had_both = had_read && had_write;
		const bool same_size
			= (viewport.width == width && viewport.height == height);

		if (same_size && had_both) {
			stats.reuses++;
			return;
		}

		if (!had_both)
			stats.misses++;
		else
			stats.resizes++;

		auto unmark_target = [&] (const Handle handle) {
			if (auto* record
				= static_cast<TextureRecord*> (storage.try_get (handle));
				record && record->is_target) {
				record->is_target = false;
				if (stats.live_targets > 0)
					stats.live_targets--;
			}
		};

		if (had_read) {
			unmark_target (viewport.read ());
			texture_pool.release (state, viewport.read ());
		}
		if (had_write) {
			unmark_target (viewport.write ());
			texture_pool.release (state, viewport.write ());
		}

		viewport.reset ();
		viewport.width = width;
		viewport.height = height;

		viewport.at (0) = texture_pool.acquire (state);
		viewport.at (1) = texture_pool.acquire (state);

		auto mark_target = [&] (const Handle handle) {
			if (auto* record
				= static_cast<TextureRecord*> (storage.try_get (handle));
				record && !record->is_target) {
				record->is_target = true;
				stats.live_targets++;
			}
		};

		mark_target (viewport.at (0));
		mark_target (viewport.at (1));
	}

	SDL_GPUTexture* resolve_texture (const Handle handle) {
		const auto* record = static_cast<TextureRecord*> (
			storage.try_get (handle)
		);
		return record ? record->tex : nullptr;
	}

	[[nodiscard]] const TextureRegistryStats& get_pool_stats () const {
		return stats;
	}

  private:
	DenseSlotMapStorage storage;
	SDLTextureFactory texture_factory;
	Pool<TextureState, SDLTextureFactory> texture_pool;

	TextureRegistryStats stats{};
};

#endif // TEXTURE_REGISTRY_H
