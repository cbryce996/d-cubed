#ifndef STATE_H
#define STATE_H

#include <memory>

template <class T> struct IState {
	virtual ~IState () = default;

	[[nodiscard]] virtual size_t hash () const = 0;
	[[nodiscard]] virtual bool equals (const T& other) const = 0;
	[[nodiscard]] virtual std::unique_ptr<T> clone () const = 0;
};

template <class T> struct IStateKey {
	std::unique_ptr<T> state;

	explicit IStateKey (const T& state) : state (state.clone ()) {}

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
