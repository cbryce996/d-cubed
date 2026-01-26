#ifndef MEMORY_H
#define MEMORY_H

#define BLOCK_FLOATS 16
#define BLOCK_BYTES (BLOCK_FLOATS * sizeof (float))
#define ALIGNMENT 16

struct alignas (ALIGNMENT) Block {
	float data[BLOCK_FLOATS];
};

#endif // MEMORY_H
