#ifndef COMPONENT_H
#define COMPONENT_H

class IEntityComponent {
  public:
	virtual ~IEntityComponent () = default;

	virtual void on_attach () {}
	virtual void on_detach () {}

	virtual void on_load () {}
	virtual void on_unload () {}
};

#endif // COMPONENT_H
