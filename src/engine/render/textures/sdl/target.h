#ifndef TARGET_H
#define TARGET_H

#include "core/storage/lifetime/buffer.h"
#include "core/storage/storage.h"

class IStorage;

class TextureTarget {
  public:
	explicit TextureTarget (const IStorage& storage) : storage (storage) {};

	DoubleBuffer<Handle, 2> buffer;

	int width = 0;
	int height = 0;

	[[nodiscard]] bool valid () const {
		return width > 0 && height > 0
			   && buffer.valid ([&] (const Handle handle) {
					  return storage.valid (handle);
				  });
	}

	[[nodiscard]] Handle write () const { return buffer.write (); }
	[[nodiscard]] Handle read () const { return buffer.read (); }

	[[nodiscard]] float aspect_ratio () const {
		return (height > 0)
				   ? (static_cast<float> (width) / static_cast<float> (height))
				   : 1.0f;
	}

	void swap () { buffer.swap (); }

	void reset () {
		buffer.reset (Handle{0xFFFFFFFFu, 0});
		width = height = 0;
	}

	Handle& at (const std::size_t i) { return buffer.at (i); }
	[[nodiscard]] const Handle& at (const std::size_t i) const {
		return buffer.at (i);
	}

  private:
	const IStorage& storage;
};

#endif // TARGET_H
