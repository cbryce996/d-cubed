#ifndef CACHE_H
#define CACHE_H

#include <memory>
#include <ranges>
#include <unordered_map>

#include "core/storage/state.h"
#include "core/storage/storage.h"

template <class State, class Factory> class Cache {
  public:
	explicit Cache (std::shared_ptr<Factory> factory)
		: factory (std::move (factory)) {}

	Handle get_or_create (const State& state) {
		IStateKey<State> key (state);

		if (const auto it = instances.find (key); it != instances.end ())
			return it->second;

		Handle created = factory->create (state);
		instances.emplace (std::move (key), created);
		return created;
	}

	void clear () {
		for (const auto& handle : instances | std::views::values) {
			factory->destroy (handle);
		}
		instances.clear ();
	}

  private:
	std::shared_ptr<Factory> factory;
	std::unordered_map<
		IStateKey<State>, Handle, typename IStateKey<State>::Hash,
		typename IStateKey<State>::Equals>
		instances;
};

#endif // CACHE_H
