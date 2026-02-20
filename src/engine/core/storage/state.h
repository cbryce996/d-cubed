#ifndef STATE_H
#define STATE_H

#include <memory>

template <class State> struct IState {
	virtual ~IState () = default;

	[[nodiscard]] virtual size_t hash () const = 0;
	[[nodiscard]] virtual bool equals (const State& other) const = 0;
	[[nodiscard]] virtual std::unique_ptr<State> clone () const = 0;
};

template <class StateKey> struct IStateKey {
	std::unique_ptr<StateKey> state;

	explicit IStateKey (const StateKey& state) : state (state.clone ()) {}

	IStateKey (IStateKey&&) noexcept = default;
	IStateKey& operator= (IStateKey&&) noexcept = default;
	IStateKey (const IStateKey&) = delete;
	IStateKey& operator= (const IStateKey&) = delete;

	struct Hash {
		size_t operator() (const IStateKey& key) const noexcept {
			return key.state->hash ();
		}
	};

	struct Equals {
		bool operator() (
			const IStateKey& key_a, const IStateKey& key_b
		) const noexcept {
			return key_a.state->equals (*key_b.state);
		}
	};
};

#endif // STATE_H
