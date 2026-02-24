#ifndef SINGLE_H
#define SINGLE_H
#include <cstddef>

template <class T> class SingleBuffer {
  public:
	T value{};

	bool valid (auto&& pred) const { return pred (value); }

	const T& read () const { return value; }
	const T& write () const { return value; }

	void swap () {}

	void reset (const T& invalid) { value = invalid; }

	T& at (const std::size_t i) {
		(void)i;
		return value;
	}
	const T& at (const std::size_t i) const {
		(void)i;
		return value;
	}
};

#endif // SINGLE_H
