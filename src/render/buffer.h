#ifndef BUFFERS_H
#define BUFFERS_H

#include <string>
#include <unordered_map>

#include "SDL3/SDL_gpu.h"
#include "mesh.h"

constexpr size_t UNIFORM_BUFFER_SIZE = 512 * 1024;
constexpr size_t TRANSFER_BUFFER_SIZE = 1024 * 1024;

struct BufferConfig {
	size_t size;
	SDL_GPUBufferUsageFlags usage;
};

struct TransferBufferConfig {
	size_t size;
	SDL_GPUTransferBufferUsage usage;
};

enum class BufferType {
	Vertex,
	Index,
	Uniform,
	TransferUpload,
	TransferDownload
};

struct CPUBuffer {
	BufferType type;

	SDL_GPUTransferBuffer* buffer = nullptr;

	bool mapped = false;
	void* mapped_buffer = nullptr;
};

struct GPUBuffer {
	BufferType type;

	SDL_GPUBuffer* buffer = nullptr;
};

struct Buffer {
	std::string name;
	size_t size;

	CPUBuffer cpu_buffer;
	GPUBuffer gpu_buffer;
};

class BufferManager {
   public:
	explicit BufferManager(SDL_GPUDevice* device);
	~BufferManager();

	// Refactor to FrameContext object
	SDL_GPUCommandBuffer* command_buffer = nullptr;
	SDL_GPUTexture* depth_texture = nullptr;
	SDL_GPUTexture* swap_chain_texture = nullptr;

	Buffer* get_buffer(const std::string& name);
	Buffer* get_or_create_buffer(const Drawable* drawable);

	void add_buffer(Buffer& buffer);

	[[nodiscard]] SDL_GPUBuffer* create_buffer(
		BufferConfig buffer_config
	) const;
	[[nodiscard]] SDL_GPUTransferBuffer* create_transfer_buffer(
		TransferBufferConfig buffer_config
	) const;

	void copy(
		const Mesh* mesh,
		Buffer* buffer
	) const;
	void upload(const Buffer* buffer) const;

   private:
	SDL_GPUDevice* device = nullptr;

	std::unordered_map<std::string, Buffer> buffers;
};

#endif	// BUFFERS_H
