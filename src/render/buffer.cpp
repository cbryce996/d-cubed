#include "buffer.h"

#include "SDL3/SDL_gpu.h"
#include "SDL3/SDL_log.h"
#include "mesh.h"

BufferManager::BufferManager(SDL_GPUDevice* device) : device(device) {}

BufferManager::~BufferManager() = default;

Buffer* BufferManager::get_buffer(const std::string& name) {
	Buffer* buffer = buffers.contains(name) ? &buffers[name] : nullptr;
	if (!buffer) {
		SDL_LogError(SDL_LOG_CATEGORY_RENDER, "Buffer not found.");
		return nullptr;
	}
	return buffer;
}

void BufferManager::add_buffer(Buffer& buffer) {
	buffers.emplace(buffer.name, buffer);
}

SDL_GPUBuffer* BufferManager::create_buffer(const BufferConfig buffer_config) const {
	SDL_GPUBufferCreateInfo buffer_create_info{};
	buffer_create_info.usage = buffer_config.usage;
	buffer_create_info.size = buffer_config.size;

	return SDL_CreateGPUBuffer(device, &buffer_create_info);
}

SDL_GPUTransferBuffer* BufferManager::create_transfer_buffer(const TransferBufferConfig buffer_config) const {
	SDL_GPUTransferBufferCreateInfo buffer_create_info{};
	buffer_create_info.usage = buffer_config.usage;
	buffer_create_info.size = buffer_config.size;

	return SDL_CreateGPUTransferBuffer(device, &buffer_create_info);
}

void BufferManager::upload(
	const Buffer* buffer
) const {
	SDL_GPUCopyPass* copy = SDL_BeginGPUCopyPass(command_buffer);

	const SDL_GPUTransferBufferLocation transfer_loc{buffer->cpu_buffer.buffer, 0};
	const SDL_GPUBufferRegion buffer_region{buffer->gpu_buffer.buffer, 0, static_cast<Uint32>(buffer->size)};

	SDL_UploadToGPUBuffer(copy, &transfer_loc, &buffer_region, true);
	SDL_EndGPUCopyPass(copy);
}

void BufferManager::copy(
	const Mesh* mesh,
	Buffer* buffer
) const {
	if (mesh->vertex_size > buffer->size) {
		SDL_LogError(SDL_LOG_CATEGORY_RENDER, "Mesh too large to copy into buffer.");
		return;
	}

	void* mapped_buffer = SDL_MapGPUTransferBuffer(device, buffer->cpu_buffer.buffer, true);

	if (!mapped_buffer) {
		SDL_LogError(SDL_LOG_CATEGORY_RENDER, "Failed to map vertex buffer: %s", SDL_GetError());
		return;
	}

	buffer->cpu_buffer.mapped = true;
	buffer->cpu_buffer.mapped_buffer = mapped_buffer;

	memcpy(mapped_buffer, mesh->vertex_data, mesh->vertex_size);

	SDL_UnmapGPUTransferBuffer(device, buffer->cpu_buffer.buffer);
}
