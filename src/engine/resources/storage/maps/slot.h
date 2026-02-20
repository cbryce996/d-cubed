#ifndef SLOT_H
#define SLOT_H

#include <cstddef>
#include <vector>

#include "resources/storage/storage.h"

class DenseSlotMap final : public IStorage {
  public:
	DenseSlotMap () = default;
	~DenseSlotMap () override = default;

	Handle allocate (std::size_t size, std::size_t align) override;
	bool free (Handle handle) override;

	[[nodiscard]] bool valid (Handle handle) const override;

	void* try_get (Handle handle) override;
	[[nodiscard]] const void* try_get (Handle handle) const override;

	void clear ();

  private:
	struct Slot {
		uint32_t gen = 0;
		bool occupied = false;
	};

	std::vector<Slot> slots;
	std::vector<uint32_t> free_ids;
	std::vector<std::byte> data;

	std::size_t record_size = 0;
	std::size_t record_align = 0;

	Slot* slot_if_valid (Handle handle);
	[[nodiscard]] const Slot* slot_if_valid (Handle handle) const;
};

#endif // SLOT_H
