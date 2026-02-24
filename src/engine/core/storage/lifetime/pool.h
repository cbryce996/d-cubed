#ifndef POOL_H
#define POOL_H

#include <ranges>
#include <unordered_map>
#include <vector>

#include "core/storage/state.h"
#include "core/storage/storage.h"

struct PoolStats {
	uint64_t acquire_calls = 0;
	uint64_t release_calls = 0;

	uint64_t hits = 0;
	uint64_t misses = 0;

	uint64_t creates = 0;
	uint64_t destroys = 0;

	uint32_t live_buckets = 0;
	uint32_t peak_buckets = 0;

	uint64_t free_handles = 0;
	uint64_t peak_free_handles = 0;

	uint64_t clear_calls = 0;
};

struct PoolBucketStats {
	uint64_t hits = 0;
	uint64_t misses = 0;
	uint64_t releases = 0;

	uint32_t free = 0;
	uint32_t peak_free = 0;
};

template <class State, class Factory> class Pool {
  public:
	explicit Pool (Factory& factory) : factory (factory) {}

	Handle acquire (const State& state) {
		stats.acquire_calls++;

		IStateKey<State> key (state);
		auto [it, inserted] = buckets.try_emplace (std::move (key), Bucket{});

		if (inserted) {
			stats.live_buckets++;
			if (stats.live_buckets > stats.peak_buckets)
				stats.peak_buckets = stats.live_buckets;
		}

		Bucket& bucket = it->second;

		if (!bucket.free.empty ()) {
			const Handle handle = bucket.free.back ();
			bucket.free.pop_back ();

			stats.hits++;
			++bucket.stats.hits;

			stats.free_handles--;
			bucket.stats.free = static_cast<uint32_t> (bucket.free.size ());

			return handle;
		}

		stats.misses++;
		++bucket.stats.misses;

		stats.creates++;
		return factory.create (state);
	}

	void release (const State& state, const Handle handle) {
		stats.release_calls++;

		IStateKey<State> key (state);
		auto [it, inserted] = buckets.try_emplace (std::move (key), Bucket{});

		if (inserted) {
			stats.live_buckets++;
			if (stats.live_buckets > stats.peak_buckets)
				stats.peak_buckets = stats.live_buckets;
		}

		Bucket& bucket = it->second;
		bucket.free.push_back (handle);

		++bucket.stats.releases;
		stats.free_handles++;
		if (stats.free_handles > stats.peak_free_handles)
			stats.peak_free_handles = stats.free_handles;

		bucket.stats.free = static_cast<uint32_t> (bucket.free.size ());
		if (bucket.stats.free > bucket.stats.peak_free)
			bucket.stats.peak_free = bucket.stats.free;
	}

	void clear () {
		stats.clear_calls++;

		for (auto& bucket : buckets | std::views::values) {
			for (const auto handle : bucket.free) {
				factory.destroy (handle);
				stats.destroys++;
			}
		}

		buckets.clear ();

		stats.free_handles = 0;
		stats.live_buckets = 0;
	}

	[[nodiscard]] const PoolStats& get_stats () const { return stats; }

	[[nodiscard]] const PoolBucketStats*
	get_bucket_stats (const State& state) const {
		IStateKey<State> key (state);
		auto it = buckets.find (key);
		if (it == buckets.end ())
			return nullptr;
		return &it->second.stats;
	}

	[[nodiscard]] uint32_t get_bucket_free_count (const State& state) const {
		IStateKey<State> key (state);
		auto it = buckets.find (key);
		if (it == buckets.end ())
			return 0;
		return static_cast<uint32_t> (it->second.free.size ());
	}

  private:
	struct Bucket {
		std::vector<Handle> free;
		PoolBucketStats stats{};
	};

	Factory& factory;

	std::unordered_map<
		IStateKey<State>, Bucket, typename IStateKey<State>::Hash,
		typename IStateKey<State>::Equals>
		buckets;

	PoolStats stats{};
};

#endif // POOL_H