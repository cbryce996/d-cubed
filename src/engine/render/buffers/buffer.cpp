#include "buffer.h"

#include "SDL3/SDL_gpu.h"
#include "SDL3/SDL_log.h"
#include "assets/mesh/mesh.h"
#include "render/material.h"
#include "render/render.h"

BufferManager::BufferManager (SDL_GPUDevice* device) : device (device) {}

BufferManager::~BufferManager () = default;

Buffer* BufferManager::get_buffer (const std::string& name) {
	Buffer* buffer = buffers.contains (name) ? &buffers[name] : nullptr;
	if (!buffer) {
		SDL_LogError (SDL_LOG_CATEGORY_RENDER, "Buffer not found.");
		return nullptr;
	}
	return buffer;
}

Buffer* BufferManager::get_or_create_vertex_buffer (const MeshInstance& mesh) {
	const std::string key = mesh.name + "_vertex";

	Buffer* buffer = buffers.contains (key) ? &buffers[key] : nullptr;
	if (buffer)
		return buffer;

	const size_t raw_size = mesh.gpu_state.vertices.size () * sizeof (Block);

	assert (raw_size > 0);
	assert (raw_size % ALIGNMENT == 0);
	assert (sizeof (Block) % ALIGNMENT == 0);

	buffer = new Buffer{};
	buffer->name = key;
	buffer->size = raw_size;

	buffer->gpu_buffer.buffer = create_buffer (
		{.usage = SDL_GPU_BUFFERUSAGE_VERTEX, .size = buffer->size}
	);
	buffer->cpu_buffer.buffer = create_transfer_buffer (
		{.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD, .size = buffer->size}
	);

	assert (buffer->gpu_buffer.buffer);
	assert (buffer->cpu_buffer.buffer);

	buffers.emplace (key, *buffer);
	return buffer;
}

Buffer* BufferManager::get_or_create_index_buffer (const MeshInstance& mesh) {
	const std::string key = mesh.name + "_index";

	Buffer* buffer = buffers.contains (key) ? &buffers[key] : nullptr;
	if (buffer)
		return buffer;

	const size_t raw_size = mesh.gpu_state.indices.size () * sizeof (Block);

	assert (raw_size > 0);
	assert (raw_size % 4 == 0);

	buffer = new Buffer{};
	buffer->name = key;
	buffer->size = raw_size;

	buffer->gpu_buffer.buffer = create_buffer (
		{.usage = SDL_GPU_BUFFERUSAGE_INDEX, .size = buffer->size}
	);
	buffer->cpu_buffer.buffer = create_transfer_buffer (
		{.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD, .size = buffer->size}
	);

	assert (buffer->gpu_buffer.buffer);
	assert (buffer->cpu_buffer.buffer);

	buffers.emplace (key, *buffer);
	return buffer;
}

Buffer*
BufferManager::get_or_create_instance_buffer (const Drawable& drawable) {
	const std::string key
		= "instance_" + drawable.mesh->name + "_" + drawable.material->name
		  + "_"
		  + std::to_string (
			  reinterpret_cast<uintptr_t> (&drawable.instance_blocks)
		  );

	Buffer* buffer = buffers.contains (key) ? &buffers[key] : nullptr;
	if (buffer)
		return buffer;

	const size_t raw_size = drawable.instance_blocks.size () * sizeof (Block);

	assert (raw_size > 0);
	assert (raw_size % ALIGNMENT == 0);
	assert (sizeof (Block) % ALIGNMENT == 0);

	buffer = new Buffer{};
	buffer->name = key;
	buffer->size = raw_size;

	buffer->gpu_buffer.buffer = create_buffer (
		{.usage = SDL_GPU_BUFFERUSAGE_GRAPHICS_STORAGE_READ,
		 .size = buffer->size}
	);
	buffer->cpu_buffer.buffer = create_transfer_buffer (
		{.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD, .size = buffer->size}
	);

	assert (buffer->gpu_buffer.buffer);
	assert (buffer->cpu_buffer.buffer);

	buffers.emplace (key, *buffer);
	return buffer;
}

SDL_GPUBuffer*
BufferManager::create_buffer (const BufferConfig buffer_config) const {
	SDL_GPUBufferCreateInfo buffer_create_info{};
	buffer_create_info.usage = buffer_config.usage;
	buffer_create_info.size = buffer_config.size;

	return SDL_CreateGPUBuffer (device, &buffer_create_info);
}

SDL_GPUTransferBuffer* BufferManager::create_transfer_buffer (
	const TransferBufferConfig buffer_config
) const {
	SDL_GPUTransferBufferCreateInfo buffer_create_info{};
	buffer_create_info.usage = buffer_config.usage;
	buffer_create_info.size = buffer_config.size;

	return SDL_CreateGPUTransferBuffer (device, &buffer_create_info);
}

void BufferManager::upload (const Buffer& buffer) const {
	SDL_GPUCopyPass* copy = SDL_BeginGPUCopyPass (command_buffer);

	const SDL_GPUTransferBufferLocation transfer_loc{
		buffer.cpu_buffer.buffer, 0
	};
	const SDL_GPUBufferRegion buffer_region{
		buffer.gpu_buffer.buffer, 0, static_cast<Uint32> (buffer.size)
	};

	SDL_UploadToGPUBuffer (copy, &transfer_loc, &buffer_region, true);
	SDL_EndGPUCopyPass (copy);
}

void BufferManager::write (
	const void* data, const size_t size, Buffer& buffer
) const {
	assert (data);
	assert (size > 0);
	assert (size <= buffer.size);
	// assert (size % ALIGNMENT == 0); TODO: Compare LCM for all alignment
	assert (buffer.size % ALIGNMENT == 0);

	void* mapped_buffer = SDL_MapGPUTransferBuffer (
		device, buffer.cpu_buffer.buffer, true
	);

	assert (mapped_buffer);
	assert (reinterpret_cast<uintptr_t> (mapped_buffer) % ALIGNMENT == 0);

	buffer.cpu_buffer.mapped = true;
	buffer.cpu_buffer.mapped_buffer = mapped_buffer;

	std::memcpy (mapped_buffer, data, size);

	SDL_UnmapGPUTransferBuffer (device, buffer.cpu_buffer.buffer);
}

void BufferManager::push_uniforms (
	const std::vector<UniformBinding>& uniform_bindings
) const {
	assert (!uniform_bindings.empty ());

	for (const auto& [name, slot, data, size, stage] : uniform_bindings) {
		assert (data);
		assert (size > 0);
		assert (size % BLOCK_BYTES == 0);
		assert (reinterpret_cast<uintptr_t> (data) % ALIGNMENT == 0);

		if (stage == ShaderStage::Vertex) {
			SDL_PushGPUVertexUniformData (command_buffer, slot, data, size);
		}

		if (stage == ShaderStage::Fragment) {
			SDL_PushGPUFragmentUniformData (command_buffer, slot, data, size);
		}
	}
}