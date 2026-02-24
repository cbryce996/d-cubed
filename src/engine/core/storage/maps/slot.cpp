#include <cassert>

#include "slot.h"

DenseSlotMapStorage::Slot*
DenseSlotMapStorage::slot_if_valid (const Handle handle) {
	if (!handle.valid () || handle.id >= slots.size ())
		return nullptr;
	Slot& slot = slots[handle.id];
	if (!slot.occupied || slot.gen != handle.generation)
		return nullptr;
	return &slot;
}

const DenseSlotMapStorage::Slot*
DenseSlotMapStorage::slot_if_valid (const Handle handle) const {
	if (!handle.valid () || handle.id >= slots.size ())
		return nullptr;
	const Slot& slot = slots[handle.id];
	if (!slot.occupied || slot.gen != handle.generation)
		return nullptr;
	return &slot;
}

Handle DenseSlotMapStorage::allocate (
	const std::size_t size, const std::size_t align
) {
	if (record_size == 0) {
		record_size = size;
		record_align = align;
	}
	assert (size == record_size && align == record_align);

	uint32_t id;
	if (!free_ids.empty ()) {
		id = free_ids.back ();
		free_ids.pop_back ();
	} else {
		id = static_cast<uint32_t> (slots.size ());
		slots.push_back (Slot{});
		data.resize (slots.size () * record_size);
		stats.bytes_reserved = data.size ();
	}

	auto& [gen, occupied] = slots[id];
	occupied = true;

	stats.allocations++;
	stats.live++;
	if (stats.live > stats.peak_live)
		stats.peak_live = stats.live;

	stats.capacity = static_cast<uint32_t> (slots.size ());
	stats.free_list = static_cast<uint32_t> (free_ids.size ());
	stats.bytes_live = static_cast<uint64_t> (stats.live) * record_size;

	return Handle{id, gen};
}

bool DenseSlotMapStorage::free (const Handle handle) {
	Slot* slot = slot_if_valid (handle);
	if (!slot)
		return false;

	slot->occupied = false;
	slot->gen += 1;
	free_ids.push_back (handle.id);

	stats.frees++;
	if (stats.live > 0)
		stats.live--;

	stats.capacity = static_cast<uint32_t> (slots.size ());
	stats.free_list = static_cast<uint32_t> (free_ids.size ());
	stats.bytes_live = static_cast<uint64_t> (stats.live) * record_size;

	return true;
}

bool DenseSlotMapStorage::valid (const Handle handle) const {
	return slot_if_valid (handle) != nullptr;
}

void* DenseSlotMapStorage::try_get (const Handle handle) {
	if (const Slot* slot = slot_if_valid (handle); !slot)
		return nullptr;
	return &data[handle.id * record_size];
}

const void* DenseSlotMapStorage::try_get (const Handle handle) const {
	if (const Slot* slot = slot_if_valid (handle); !slot)
		return nullptr;
	return &data[handle.id * record_size];
}

void DenseSlotMapStorage::clear () {
	slots.clear ();
	free_ids.clear ();
	data.clear ();
	record_size = 0;
	record_align = 0;

	stats = {};
}