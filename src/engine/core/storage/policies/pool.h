#ifndef POOL_H
#define POOL_H

#include <memory>
#include <ranges>
#include <unordered_map>

#include "../storage.h"

template <class Key, class State, class Factory> class Pool {
  public:
	explicit Pool (std::shared_ptr<Factory> factory)
		: factory (std::move (factory)) {}

	Handle acquire(const State& state) {
		auto [it, inserted] = free_list.try_emplace(Key(state));

		if (auto& list = it->second; !list.empty()) {
			Handle h = list.back();
			list.pop_back();
			return h;
		}

		return factory->create(state);
	}

	void release (const State& state, const Handle handle) {
		Key key (state);
		free_list.emplace (std::move (key), std::vector<Handle>{})
			.first->second.push_back (handle);
	}

	void clear (IStorage& storage) {
		for (auto& list : free_list | std::views::values) {
			for (const auto handle : list)
				factory->destroy (handle, storage);
		}
		free_list.clear ();
	}

  private:
	std::shared_ptr<Factory> factory;
	std::unordered_map<
		Key, std::vector<Handle>, typename Key::Hash, typename Key::Equals>
		free_list;
};

#endif // POOL_H
