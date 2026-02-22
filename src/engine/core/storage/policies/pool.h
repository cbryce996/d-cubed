#ifndef POOL_H
#define POOL_H

#include <memory>
#include <ranges>
#include <unordered_map>

#include "core/storage/state.h"
#include "core/storage/storage.h"

template <class State, class Factory> class Pool {
  public:
	explicit Pool (Factory& factory) : factory (factory) {}

	Handle acquire (const State& state) {
		auto [it, inserted] = free_list.try_emplace (IStateKey<State> (state));

		if (auto& list = it->second; !list.empty ()) {
			const Handle handle = list.back ();
			list.pop_back ();
			return handle;
		}

		return factory.create (state);
	}

	void release (const State& state, const Handle handle) {
		IStateKey<State> key (state);
		free_list.emplace (std::move (key), std::vector<Handle>{})
			.first->second.push_back (handle);
	}

	void clear () {
		for (auto& list : free_list | std::views::values) {
			for (const auto handle : list)
				factory.destroy (handle);
		}
		free_list.clear ();
	}

  private:
	Factory& factory;
	std::unordered_map<
		IStateKey<State>, std::vector<Handle>, typename IStateKey<State>::Hash,
		typename IStateKey<State>::Equals>
		free_list;
};

#endif // POOL_H
