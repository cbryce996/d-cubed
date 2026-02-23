#ifndef TEXTURE_REGISTRY_H
#define TEXTURE_REGISTRY_H

#include "core/storage/lifetime/buffers/ring.h"
#include "core/storage/lifetime/buffers/single.h"
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

struct TextureTargetEnsureDefaults {
	static constexpr int minimum_dimension = 2;

	[[nodiscard]] static int clamp_dimension (const int value) {
		return (value > 0) ? value : minimum_dimension;
	}

	[[nodiscard]] constexpr int minimum_width () const {
		return minimum_dimension;
	}
	[[nodiscard]] constexpr int minimum_height () const {
		return minimum_dimension;
	}
};

struct ViewportEnsurer : TextureTargetEnsureDefaults {
	[[nodiscard]] constexpr std::size_t buffer_count () const { return 2; }
	[[nodiscard]] constexpr bool is_double_buffered () const { return true; }

	[[nodiscard]] TextureState
	make_state (const int requested_width, const int requested_height) const {
		TextureState desired{};
		desired.width = clamp_dimension (requested_width);
		desired.height = clamp_dimension (requested_height);
		desired.num_samplers = 1;
		desired.format = TextureFormat::RGBA8;
		desired.usage = TextureUsage::ColorTarget | TextureUsage::Sampled;
		return desired;
	}
};

struct GBufferColorTargetEnsurer : TextureTargetEnsureDefaults {
	TextureFormat format = TextureFormat::RGBA8;

	[[nodiscard]] constexpr std::size_t buffer_count () const { return 1; }
	[[nodiscard]] constexpr bool is_double_buffered () const { return false; }

	[[nodiscard]] TextureState
	make_state (const int requested_width, const int requested_height) const {
		TextureState desired{};
		desired.width = clamp_dimension (requested_width);
		desired.height = clamp_dimension (requested_height);
		desired.num_samplers = 1;
		desired.format = format;
		desired.usage = TextureUsage::ColorTarget | TextureUsage::Sampled;
		return desired;
	}
};

struct GBufferPositionEnsurer : GBufferColorTargetEnsurer {
	GBufferPositionEnsurer () { format = TextureFormat::RGBA16F; }
};

struct GBufferNormalEnsurer : GBufferColorTargetEnsurer {
	GBufferNormalEnsurer () { format = TextureFormat::RGBA16F; }
};

struct GBufferAlbedoEnsurer : GBufferColorTargetEnsurer {
	GBufferAlbedoEnsurer () { format = TextureFormat::RGBA8; }
};

class TextureRegistry {
  public:
	explicit TextureRegistry (SDL_GPUDevice* device)
		: viewport (storage, ViewportEnsurer{}),
		  gbuffer_position (storage, GBufferPositionEnsurer{}),
		  gbuffer_normal (storage, GBufferNormalEnsurer{}),
		  gbuffer_albedo (storage, GBufferAlbedoEnsurer{}),
		  texture_factory (device, storage), texture_pool (texture_factory) {}

	Target<RingBuffer<Handle, 2>, ViewportEnsurer> viewport;

	Target<SingleBuffer<Handle>, GBufferPositionEnsurer> gbuffer_position;
	Target<SingleBuffer<Handle>, GBufferNormalEnsurer> gbuffer_normal;
	Target<SingleBuffer<Handle>, GBufferAlbedoEnsurer> gbuffer_albedo;

	template <class Buffer, class Ensurer>
	void ensure_target (
		Target<Buffer, Ensurer>& target, const int requested_width,
		const int requested_height
	) {
		const int ensured_width = (requested_width > 0)
									  ? requested_width
									  : target.ensurer.minimum_width ();
		const int ensured_height = (requested_height > 0)
									   ? requested_height
									   : target.ensurer.minimum_height ();

		const bool has_read_handle = storage.valid (target.read ());
		const bool has_write_handle = storage.valid (target.write ());

		const bool has_required_handles = target.ensurer.is_double_buffered ()
											  ? (has_read_handle
												 && has_write_handle)
											  : has_read_handle;

		const bool same_size
			= (target.width == ensured_width
			   && target.height == ensured_height);

		if (same_size && has_required_handles) {
			return;
		}

		auto release_handle = [&] (const Handle handle) {
			if (!storage.valid (handle)) {
				return;
			}

			const auto* record = static_cast<TextureRecord*> (
				storage.try_get (handle)
			);
			if (!record) {
				return;
			}

			texture_pool.release (record->state, handle);
		};

		if (has_read_handle) {
			release_handle (target.read ());
		}

		if (target.ensurer.is_double_buffered () && has_write_handle) {
			release_handle (target.write ());
		}

		target.reset ();
		target.width = ensured_width;
		target.height = ensured_height;

		const TextureState desired = target.ensurer.make_state (
			ensured_width, ensured_height
		);

		const std::size_t target_buffer_count = target.ensurer.buffer_count ();
		for (std::size_t buffer_index = 0; buffer_index < target_buffer_count;
			 ++buffer_index) {
			target.at (buffer_index) = texture_pool.acquire (desired);
		}
	}

	void ensure_viewport_target (const int width, const int height) {
		ensure_target (viewport, width, height);
	}

	void ensure_gbuffer_targets (const int width, const int height) {
		ensure_target (gbuffer_position, width, height);
		ensure_target (gbuffer_normal, width, height);
		ensure_target (gbuffer_albedo, width, height);
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

		const Handle viewport_read_handle = viewport.read ();
		const Handle viewport_write_handle = viewport.write ();

		out.viewport_read_valid = storage.valid (viewport_read_handle);
		out.viewport_write_valid = storage.valid (viewport_write_handle);

		auto add_bytes = [&] (const Handle handle) {
			if (!storage.valid (handle)) {
				return;
			}

			const auto* record = static_cast<const TextureRecord*> (
				storage.try_get (handle)
			);
			if (!record) {
				return;
			}

			out.viewport_bytes += record->approx_bytes;
		};

		add_bytes (viewport_read_handle);
		add_bytes (viewport_write_handle);

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