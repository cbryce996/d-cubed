#ifndef MEMORY_H
#define MEMORY_H

#include <GLM/glm.hpp>

struct Block {
	float slots[4];
};

struct alignas (16) Collection {
	Block blocks[4];
	uint8_t count = 0;

	static constexpr uint8_t MAX_BLOCKS = 4;

	void clear () { count = 0; }

	static void push (Collection* collection, const glm::mat4& data) {
		assert (collection != nullptr);
		assert (
			collection->count == 0 && "Collection must be empty to push a mat4"
		);

		for (int i = 0; i < 4; ++i) {
			push (collection, data[i]);
		}
	}

	static void push (Collection* collection, const glm::vec4& value) {
		assert (collection != nullptr);
		assert (collection->count < MAX_BLOCKS && "Collection overflow: vec4");

		std::memcpy (
			collection->blocks[collection->count].slots, &value,
			sizeof (glm::vec4)
		);

		collection->count++;
	}

	static void push (Collection* collection, const float value) {
		assert (collection != nullptr);
		assert (collection->count < MAX_BLOCKS && "Collection overflow: float");

		glm::vec4 filler (value);
		std::memcpy (
			collection->blocks[collection->count].slots, &filler,
			sizeof (glm::vec4)
		);

		collection->count++;
	}
};

enum class VertexFormat { P_N_C_U };

struct Vertex : Collection {
	VertexFormat format = VertexFormat::P_N_C_U;
};

#endif // MEMORY_H
