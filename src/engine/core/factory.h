#ifndef CORE_FACTORY_H
#define CORE_FACTORY_H

#include "storage/storage.h"

template <class Record> class IFactory {
  public:
	explicit IFactory (IStorage& storage) : storage (storage) {}
	virtual ~IFactory () = default;

	virtual Handle create (const Record& record) = 0;
	virtual void destroy (Handle handle) = 0;

	IStorage& storage;
};

#endif // CORE_FACTORY_H
