#ifndef STORAGE_H
#define STORAGE_H

#include <cstddef>
#include <cstdint>

struct Handle {
	uint32_t id = 0;
	uint32_t generation = 0;
	[[nodiscard]] bool valid () const noexcept { return id != 0xFFFFFFFFu; }
};

class IStorage {
  public:
	virtual ~IStorage () = default;

	virtual Handle allocate (std::size_t size, std::size_t align) = 0;

	virtual bool free (Handle handle) = 0;

	[[nodiscard]] virtual bool valid (Handle handle) const = 0;

	virtual void* try_get (Handle handle) = 0;
	[[nodiscard]] virtual const void* try_get (Handle handle) const = 0;

	virtual void begin_frame (uint64_t frame) {}
	virtual void end_frame (uint64_t frame) {}
};

#endif // STORAGE_H
