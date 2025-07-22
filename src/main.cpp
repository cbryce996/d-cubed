#include "engine/engine.h"

int main(
	int argc,
	char* argv[]
) {
	Engine engine;
	engine.run();
	engine.shutdown();
	return 0;
}
