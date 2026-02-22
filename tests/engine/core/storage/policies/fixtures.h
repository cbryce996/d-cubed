#ifndef FIXTURES_H
#define FIXTURES_H

#include "core/storage/state.h"

struct FakeState final : IState<FakeState> {
	int v = 0;

	explicit FakeState (const int v = 0) : v (v) {}

	size_t hash () const override {
		return static_cast<size_t> (v) * 2654435761u;
	}

	bool equals (const FakeState& other) const override { return v == other.v; }

	std::unique_ptr<FakeState> clone () const override {
		return std::make_unique<FakeState> (*this);
	}
};

using FakeStateKey = IStateKey<FakeState>;

class FakeStorage final : public IStorage {
  public:
	Handle allocate (std::size_t, std::size_t) override { return {}; }
	bool free (Handle) override { return true; }
	bool valid (Handle) const override { return true; }
	void* try_get (Handle) override { return nullptr; }
	const void* try_get (Handle) const override { return nullptr; }
};

class FakeFactory {
  public:
	Handle create (const FakeState& state) {
		created_states.push_back (state.v);
		return Handle{static_cast<uint32_t> (++next_id), 0};
	}

	void destroy (const Handle handle) { destroyed_ids.push_back (handle.id); }

	int next_id = 0;
	std::vector<int> created_states;
	std::vector<uint32_t> destroyed_ids;
};

#endif // FIXTURES_H
