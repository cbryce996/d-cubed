#ifndef BUFFERS_H
#define BUFFERS_H

#include <string>
#include <unordered_map>

#include "SDL3/SDL_gpu.h"
#include "render/render.h"

struct Drawable;
struct MeshInstance;

struct BufferConfig {
	size_t size;
	SDL_GPUBufferUsageFlags usage;
};

struct TransferBufferConfig {
	size_t size;
	SDL_GPUTransferBufferUsage usage;
};

struct CPUBuffer {
	SDL_GPUTransferBuffer* buffer = nullptr;

	bool mapped = false;
	void* mapped_buffer = nullptr;
};

struct GPUBuffer {
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
	explicit BufferManager (SDL_GPUDevice* device);
	~BufferManager ();

	SDL_GPUCommandBuffer* command_buffer = nullptr;
	SDL_GPUTexture* depth_texture = nullptr;
	SDL_GPUTexture* swap_chain_texture = nullptr;

	SDL_GPUSampler* linear_sampler = nullptr;

	Buffer* get_buffer (const std::string& name);
	Buffer* get_or_create_vertex_buffer (const MeshInstance& mesh);
	Buffer* get_or_create_index_buffer (const MeshInstance& mesh);
	Buffer* get_or_create_instance_buffer (const Drawable& drawable);

	[[nodiscard]] SDL_GPUBuffer*
	create_buffer (BufferConfig buffer_config) const;
	[[nodiscard]] SDL_GPUTransferBuffer*
	create_transfer_buffer (TransferBufferConfig buffer_config) const;

	void write (const void* data, size_t size, Buffer& buffer) const;
	void
	push_uniforms (const std::vector<UniformBinding>& uniform_bindings) const;
	void upload (const Buffer& buffer) const;

  private:
	SDL_GPUDevice* device = nullptr;

	std::unordered_map<std::string, Buffer> buffers;
};

#endif // BUFFERS_H
