#ifndef MEMORY_H
#define MEMORY_H

#define BASE_BLOCK_SIZE 4
#define BASE_COLLECTION_SIZE 4
#define UNIFORM_ALIGNMENT 16

#include <GLM/glm.hpp>

struct alignas (UNIFORM_ALIGNMENT) Block {
	float data[BASE_BLOCK_SIZE * BASE_COLLECTION_SIZE];

	void clear () { std::memset (&data, 0, sizeof (data)); }

	void write (const size_t slot, const glm::vec4& v) {
		assert (slot < BASE_COLLECTION_SIZE);
		std::memcpy (&data[slot * 4], &v, sizeof (glm::vec4));
	}

	static Block from (const glm::vec4& v) {
		Block b{};
		b.clear ();
		b.write (0, v);
		return b;
	}

	static Block from (const glm::mat4& m) {
		Block b{};
		b.clear ();

		b.write (0, m[0]);
		b.write (1, m[1]);
		b.write (2, m[2]);
		b.write (3, m[3]);

		return b;
	}

	[[nodiscard]] glm::vec4 to_cpu (const size_t slot = 0) const {
		assert (slot < BASE_COLLECTION_SIZE);
		glm::vec4 v;
		std::memcpy (&v, &data[slot * 4], BASE_BLOCK_SIZE * sizeof (float));
		return v;
	}

	[[nodiscard]] glm::mat4 to_cpu () const {
		glm::mat4 m;
		m[0] = to_cpu (0);
		m[1] = to_cpu (1);
		m[2] = to_cpu (2);
		m[3] = to_cpu (3);
		return m;
	}
};

#endif // MEMORY_H
