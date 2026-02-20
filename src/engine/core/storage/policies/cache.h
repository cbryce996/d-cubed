#ifndef CACHE_H
#define CACHE_H

#include <memory>
#include <ranges>
#include <unordered_map>

#include "../storage.h"

template <class Key, class State, class Factory> class Cache {
  public:
	explicit Cache (std::shared_ptr<Factory> factory)
		: factory (std::move (factory)) {}

	Handle get_or_create (const State& state) {
		Key key (state);

		if (const auto it = instances.find (key); it != instances.end ())
			return it->second;

		Handle created = factory->create (state);
		instances.emplace (std::move (key), created);
		return created;
	}

	void clear (IStorage& storage) {
		for (const auto& handle : instances | std::views::values) {
			factory->destroy (handle, storage);
		}
		instances.clear ();
	}

  private:
	std::shared_ptr<Factory> factory;
	std::unordered_map<Key, Handle, typename Key::Hash, typename Key::Equals>
		instances;
};

#endif // CACHE_H
