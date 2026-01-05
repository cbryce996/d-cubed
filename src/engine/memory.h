#ifndef MEMORY_H
#define MEMORY_H

#define BASE_BLOCK_SIZE 4
#define BASE_COLLECTION_SIZE 4
#define UNIFORM_ALIGNMENT 16

#include <GLM/glm.hpp>

struct alignas(UNIFORM_ALIGNMENT) Block {
	float blocks[BASE_BLOCK_SIZE * BASE_COLLECTION_SIZE];
};

struct Collection {
	Block storage{};
	uint8_t count = 0;

	static constexpr uint8_t MAX_BLOCKS = 4;

	void clear () {
		count = 0;
		std::memset (storage.blocks, 0, sizeof (Block));
	}

	void push (const glm::vec4& value) {
		assert (count < MAX_BLOCKS && "Collection overflow");
		// Each vec4 takes 4 float slots
		std::memcpy (&storage.blocks[count * 4], &value, sizeof (glm::vec4));
		count++;
	}

	void push (const glm::mat4& m) {
		assert (count == 0 && "Collection must be empty for mat4");
		for (int i = 0; i < 4; ++i)
			push (m[i]);
	}

	void push (const float value) { push (glm::vec4 (value)); }
};

#endif // MEMORY_H
