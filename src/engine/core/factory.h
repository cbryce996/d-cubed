#ifndef FACTORY_H
#define FACTORY_H

struct Handle;
class IStorage;

template <class State, class Storage> class IFactory {
  public:
	explicit IFactory (Storage& storage) : storage (storage) {};
	virtual ~IFactory () = default;

	virtual Handle create (const State& state) = 0;
	virtual void destroy (Handle handle, Storage& storage) = 0;

  private:
	Storage& storage;
};

#endif // FACTORY_H
