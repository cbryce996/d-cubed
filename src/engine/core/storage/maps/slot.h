#ifndef SLOT_H
#define SLOT_H

#include <cstddef>
#include <vector>

#include "core/storage/storage.h"

struct DenseSlotMapStats {
	uint64_t allocations = 0;
	uint64_t frees = 0;
	uint32_t live = 0;
	uint32_t peak_live = 0;

	uint32_t capacity = 0;
	uint32_t free_list = 0;

	uint64_t bytes_reserved = 0;
	uint64_t bytes_live = 0;
};

class DenseSlotMapStorage final : public IStorage {
  public:
	DenseSlotMapStorage () = default;
	~DenseSlotMapStorage () override = default;

	Handle allocate (std::size_t size, std::size_t align) override;
	bool free (Handle handle) override;

	[[nodiscard]] bool valid (Handle handle) const override;

	void* try_get (Handle handle) override;
	[[nodiscard]] const void* try_get (Handle handle) const override;

	void clear ();

	[[nodiscard]] const DenseSlotMapStats& get_stats () const { return stats; }

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

	DenseSlotMapStats stats{};

	Slot* slot_if_valid (Handle handle);
	[[nodiscard]] const Slot* slot_if_valid (Handle handle) const;
};

#endif // SLOT_H
