#ifndef BUFFER_H
#define BUFFER_H

#include <future>

template <class T, std::size_t N = 2> class DoubleBuffer {
	static_assert (N >= 2, "DoubleBuffer needs at least 2 buffers.");

  public:
	using value_type = T;
	static constexpr std::size_t BufferCount = N;

	DoubleBuffer () = default;
	explicit DoubleBuffer (const T resource) { reset (resource); }

	[[nodiscard]] T& write () { return buffers[write_index]; }
	[[nodiscard]] const T& write () const { return buffers[write_index]; }

	[[nodiscard]] T& read () { return buffers[read_index]; }
	[[nodiscard]] const T& read () const { return buffers[read_index]; }

	[[nodiscard]] T& at (std::size_t i) { return buffers[i]; }
	[[nodiscard]] const T& at (std::size_t i) const { return buffers[i]; }

	void swap () { std::swap (write_index, read_index); }

	void reset () {
		for (auto& buffer : buffers)
			buffer = T{};
		write_index = 0;
		read_index = (N > 1) ? 1 : 0;
	}

	void reset (const T& resource) {
		for (auto& buffer : buffers)
			buffer = resource;
		write_index = 0;
		read_index = (N > 1) ? 1 : 0;
	}

	[[nodiscard]] std::size_t write_idx () const { return write_index; }
	[[nodiscard]] std::size_t read_idx () const { return read_index; }

	template <class Predicate>
	[[nodiscard]] bool valid (Predicate predicate) const {
		for (const auto& buffer : buffers) {
			if (!predicate (buffer))
				return false;
		}
		return true;
	}

  private:
	T buffers[N] = {};
	std::size_t write_index = 0;
	std::size_t read_index = (N > 1) ? 1 : 0;
};

#endif // BUFFER_H
