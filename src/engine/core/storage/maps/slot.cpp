#include <cassert>

#include "slot.h"

DenseSlotMap::Slot* DenseSlotMap::slot_if_valid (const Handle handle) {
	if (!handle.valid () || handle.id >= slots.size ())
		return nullptr;
	Slot& slot = slots[handle.id];
	if (!slot.occupied || slot.gen != handle.generation)
		return nullptr;
	return &slot;
}

const DenseSlotMap::Slot*
DenseSlotMap::slot_if_valid (const Handle handle) const {
	if (!handle.valid () || handle.id >= slots.size ())
		return nullptr;
	const Slot& slot = slots[handle.id];
	if (!slot.occupied || slot.gen != handle.generation)
		return nullptr;
	return &slot;
}

Handle
DenseSlotMap::allocate (const std::size_t size, const std::size_t align) {
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
		data.resize ((slots.size ()) * record_size);
	}

	Slot& slot = slots[id];
	slot.occupied = true;
	return Handle{id, slot.gen};
}

bool DenseSlotMap::free (const Handle handle) {
	Slot* slot = slot_if_valid (handle);
	if (!slot)
		return false;

	slot->occupied = false;
	slot->gen += 1;
	free_ids.push_back (handle.id);
	return true;
}

bool DenseSlotMap::valid (const Handle handle) const {
	return slot_if_valid (handle) != nullptr;
}

void* DenseSlotMap::try_get (const Handle handle) {
	if (const Slot* slot = slot_if_valid (handle); !slot)
		return nullptr;
	return &data[handle.id * record_size];
}

const void* DenseSlotMap::try_get (const Handle handle) const {
	if (const Slot* slot = slot_if_valid (handle); !slot)
		return nullptr;
	return &data[handle.id * record_size];
}

void DenseSlotMap::clear () {
	slots.clear ();
	free_ids.clear ();
	data.clear ();
	record_size = 0;
	record_align = 0;
}