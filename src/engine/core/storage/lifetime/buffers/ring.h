#ifndef BUFFER_H
#define BUFFER_H

#include <future>

template <class T, std::size_t N> class RingBuffer {
  public:
	std::array<T, N> values{};
	std::size_t index = 0;

	bool valid (auto&& pred) const {
		for (const auto& v : values)
			if (!pred (v))
				return false;
		return true;
	}

	const T& read () const { return values[index]; }
	const T& write () const { return values[index]; }

	void swap () { index = (index + 1) % N; }

	void reset (const T& invalid) {
		values.fill (invalid);
		index = 0;
	}

	T& at (std::size_t i) { return values[i]; }
	const T& at (std::size_t i) const { return values[i]; }
};

#endif // BUFFER_H
