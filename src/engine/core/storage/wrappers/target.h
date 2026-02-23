#ifndef TARGET_H
#define TARGET_H

#include "core/storage/storage.h"

struct Handle;

template <class Buffer> class Target {
  public:
	explicit Target (const IStorage& storage) : storage (storage) {}

	Buffer buffer;

	int width = 0;
	int height = 0;

	[[nodiscard]] bool valid () const {
		return width > 0 && height > 0 && buffer.valid ([&] (const Handle h) {
			return storage.valid (h);
		});
	}

	[[nodiscard]] Handle write () const { return buffer.write (); }
	[[nodiscard]] Handle read () const { return buffer.read (); }

	[[nodiscard]] float aspect_ratio () const {
		return (height > 0)
				   ? static_cast<float> (width) / static_cast<float> (height)
				   : 1.0f;
	}

	void swap () { buffer.swap (); }

	void reset () {
		buffer.reset (invalid_handle ());
		width = height = 0;
	}

	Handle& at (std::size_t i) { return buffer.at (i); }
	const Handle& at (std::size_t i) const { return buffer.at (i); }

  private:
	static constexpr Handle invalid_handle () { return Handle{0xFFFFFFFFu, 0}; }

	const IStorage& storage;
};

#endif // TARGET_H
