#ifndef TEXTURE_REGISTRY_H
#define TEXTURE_REGISTRY_H

#include "core/storage/lifetime/buffers/ring.h"
#include "core/storage/lifetime/pool.h"
#include "core/storage/maps/slot.h"
#include "core/storage/record.h"
#include "core/storage/wrappers/target.h"
#include "sdl/factory.h"
#include "texture.h"

struct TextureDebugSnapshot {
	DenseSlotMapStats storage{};
	PoolStats pool{};

	int viewport_width = 0;
	int viewport_height = 0;
	bool viewport_read_valid = false;
	bool viewport_write_valid = false;

	uint64_t viewport_bytes = 0;
};

class TextureRegistry {
  public:
	explicit TextureRegistry (SDL_GPUDevice* device)
		: viewport (storage), texture_factory (device, storage),
		  texture_pool (texture_factory) {}

	Target<RingBuffer<Handle, 2>> viewport;

	void ensure_viewport_target (const int new_width, const int new_height) {
		const int width = (new_width > 0) ? new_width : 2;
		const int height = (new_height > 0) ? new_height : 2;

		TextureState desired{};
		desired.width = width;
		desired.height = height;
		desired.num_samplers = 1;
		desired.format = TextureFormat::RGBA8;
		desired.usage = TextureUsage::ColorTarget | TextureUsage::Sampled;

		const bool had_read = storage.valid (viewport.read ());
		const bool had_write = storage.valid (viewport.write ());
		const bool had_both = had_read && had_write;
		const bool same_size
			= (viewport.width == width && viewport.height == height);

		if (same_size && had_both)
			return;

		auto release_handle = [&] (const Handle handle) {
			if (!storage.valid (handle))
				return;
			const auto* record = static_cast<TextureRecord*> (
				storage.try_get (handle)
			);
			if (!record)
				return;
			texture_pool.release (record->state, handle);
		};

		if (had_read)
			release_handle (viewport.read ());
		if (had_write)
			release_handle (viewport.write ());

		viewport.reset ();
		viewport.width = width;
		viewport.height = height;

		viewport.at (0) = texture_pool.acquire (desired);
		viewport.at (1) = texture_pool.acquire (desired);
	}

	SDL_GPUTexture* resolve_texture (const Handle handle) {
		const auto* record = static_cast<TextureRecord*> (
			storage.try_get (handle)
		);
		return record ? record->tex : nullptr;
	}

	[[nodiscard]] TextureDebugSnapshot get_debug_snapshot () const {
		TextureDebugSnapshot out{};
		out.storage = storage.get_stats ();
		out.pool = texture_pool.get_stats ();

		out.viewport_width = viewport.width;
		out.viewport_height = viewport.height;

		const Handle read = viewport.read ();
		const Handle write = viewport.write ();

		out.viewport_read_valid = storage.valid (read);
		out.viewport_write_valid = storage.valid (write);

		auto add_bytes = [&] (const Handle handle) {
			if (!storage.valid (handle))
				return;
			const auto* record = static_cast<const TextureRecord*> (
				storage.try_get (handle)
			);
			if (!record)
				return;
			out.viewport_bytes += record->approx_bytes;
		};

		add_bytes (read);
		add_bytes (write);

		return out;
	}

	[[nodiscard]] const DenseSlotMapStats& storage_stats () const {
		return storage.get_stats ();
	}
	[[nodiscard]] const PoolStats& pool_stats () const {
		return texture_pool.get_stats ();
	}

  private:
	DenseSlotMapStorage storage;
	SDLTextureFactory texture_factory;
	Pool<TextureState, SDLTextureFactory> texture_pool;
};

#endif // TEXTURE_REGISTRY_H