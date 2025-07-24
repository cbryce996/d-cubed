#include "render.h"

#include <SDL3/SDL.h>
#include <SDL3/SDL_gpu.h>
#include <iostream>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>

// Triangle in normalized device space (X/Y from -1 to 1)
constexpr Vertex triangle_vertices[] = {
    { glm::vec3( 0.0f,  0.5f, 0.0f) },
    { glm::vec3( 0.5f, -0.5f, 0.0f) },
    { glm::vec3(-0.5f, -0.5f, 0.0f) }
};


RenderManager::RenderManager(SDL_GPUDevice* device, SDL_Window* window)
    : device(device), window(window) {
    load_shaders();
    setup_vertex_format();
    create_pipeline();
    create_buffers();
}

RenderManager::~RenderManager() = default;

// Shader Compilation
void RenderManager::load_shaders() {
    vertex_shader   = load_shader("../shaders/msl/triangle_buffer_uniform.metal", SDL_GPU_SHADERSTAGE_VERTEX, "vert_main");
    fragment_shader = load_shader("../shaders/msl/triangle_buffer_uniform.metal", SDL_GPU_SHADERSTAGE_FRAGMENT, "frag_main");
}

// Vertex Layout Setup
void RenderManager::setup_vertex_format() {
    // One buffer: position only, 3 floats (x/y/z)
    vbo_desc.slot = 0;
    vbo_desc.input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX;
    vbo_desc.pitch = sizeof(Vertex);

    attr.location = 0;
    attr.buffer_slot = 0;
    attr.format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3;
    attr.offset = 0;

    vi_state.vertex_buffer_descriptions = &vbo_desc;
    vi_state.num_vertex_buffers = 1;
    vi_state.vertex_attributes = &attr;
    vi_state.num_vertex_attributes = 1;
}

// Graphics Pipeline
void RenderManager::create_pipeline() {
    // Define the format of the output (render target)
    SDL_GPUColorTargetDescription color_desc{};
    color_desc.format = SDL_GetGPUSwapchainTextureFormat(device, window);
    color_desc.blend_state = {}; // no blending

    SDL_GPUGraphicsPipelineTargetInfo target_info{};
    target_info.num_color_targets = 1;
    target_info.color_target_descriptions = &color_desc;
    target_info.has_depth_stencil_target = false;

    // Create graphics pipeline
    SDL_GPUGraphicsPipelineCreateInfo gp_info{};
    gp_info.vertex_input_state = vi_state;
    gp_info.primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST;
    gp_info.vertex_shader = vertex_shader;
    gp_info.fragment_shader = fragment_shader;
    gp_info.rasterizer_state.cull_mode = SDL_GPU_CULLMODE_NONE;
    gp_info.depth_stencil_state.enable_depth_test = false;
    gp_info.depth_stencil_state.enable_depth_write = false;
    gp_info.target_info = target_info;
    gp_info.props = 0;
    pipeline = SDL_CreateGPUGraphicsPipeline(device, &gp_info);
}

// Buffer Creation
void RenderManager::create_buffers() {
    // GPU-side buffer for drawing vertices
    SDL_GPUBufferCreateInfo buf_info{};
    buf_info.size = sizeof(triangle_vertices);
    buf_info.usage = SDL_GPU_BUFFERUSAGE_VERTEX;
    buf_info.props = 0;
    vertex_buffer = SDL_CreateGPUBuffer(device, &buf_info);

    // CPU-side transfer buffer for uploading vertex data
    SDL_GPUTransferBufferCreateInfo transfer_info{};
    transfer_info.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
    transfer_info.size = sizeof(triangle_vertices);
    transfer_info.props = 0;
    transfer_buffer = SDL_CreateGPUTransferBuffer(device, &transfer_info);

    // Upload initial geometry to the transfer buffer
    void* mapped = SDL_MapGPUTransferBuffer(device, transfer_buffer, true);
    if (mapped) {
        memcpy(mapped, triangle_vertices, sizeof(triangle_vertices));
        SDL_UnmapGPUTransferBuffer(device, transfer_buffer);
    } else {
        SDL_LogError(SDL_LOG_CATEGORY_RENDER, "Failed to map vertex buffer: %s", SDL_GetError());
    }
}

// Frame Rendering
void RenderManager::render_state(const RenderState& state) {
    // Acquire command buffer for this frame
    SDL_GPUCommandBuffer* command_buffer = SDL_AcquireGPUCommandBuffer(device);

    // Upload geometry
    SDL_GPUCopyPass* copy = SDL_BeginGPUCopyPass(command_buffer);
    SDL_GPUTransferBufferLocation transfer_loc{transfer_buffer, 0};
    SDL_GPUBufferRegion buffer_region{vertex_buffer, 0, sizeof(triangle_vertices)};
    SDL_UploadToGPUBuffer(copy, &transfer_loc, &buffer_region, true);
    SDL_EndGPUCopyPass(copy);

    // Acquire backbuffer
    SDL_GPUTexture* texture = nullptr;

    if (!SDL_WaitAndAcquireGPUSwapchainTexture(command_buffer, window, &texture, &width, &height)) {
        std::cerr << "[Render] Failed to acquire backbuffer: " << SDL_GetError() << "\n";
        return;
    }

    // Begin render pass
    const SDL_GPUColorTargetInfo target = {
        .texture = texture,
        .mip_level = 0,
        .layer_or_depth_plane = 0,
        .clear_color = {0.53f, 0.81f, 0.92f, 1.0f},
        .load_op = SDL_GPU_LOADOP_CLEAR,
        .store_op = SDL_GPU_STOREOP_STORE,
        .resolve_texture = nullptr,
        .resolve_mip_level = 0,
        .resolve_layer = 0,
        .cycle = false,
        .cycle_resolve_texture = false
    };

    SDL_GPURenderPass* pass = SDL_BeginGPURenderPass(command_buffer, &target, 1, nullptr);

    // Setup viewport
    SDL_GPUViewport viewport = {
        .x = 0,
        .y = 0,
        .w = static_cast<float>(width),
        .h = static_cast<float>(height),
        .min_depth = 0.0f,
        .max_depth = 1.0f
    };
    SDL_SetGPUViewport(pass, &viewport);

    // --- Compute MVP Matrix ---
    Uniform ubo{};
    ubo.mvp = rotate_matrix();

    // Send it to the shader via slot 0
    SDL_PushGPUVertexUniformData(command_buffer, 0, &ubo, sizeof(ubo));

    // Pipeline + vertex buffer bind
    SDL_BindGPUGraphicsPipeline(pass, pipeline);
    SDL_GPUBufferBinding bindings[1] = { { vertex_buffer, 0 } };
    SDL_BindGPUVertexBuffers(pass, 0, bindings, 1);

    // Draw call
    SDL_DrawGPUPrimitives(pass, 3, 1, 0, 0);

    // Finish
    SDL_EndGPURenderPass(pass);
    SDL_SubmitGPUCommandBuffer(command_buffer);
}

void RenderManager::update_camera(float deltaTime, const bool* keys) {
    const float speed = 0.01f;

    glm::vec3 forward = glm::normalize(camera_target - camera_position);
    glm::vec3 right   = glm::normalize(glm::cross(forward, camera_up));

    if (keys[SDL_SCANCODE_W]) camera_position += forward * speed * deltaTime;
    if (keys[SDL_SCANCODE_S]) camera_position -= forward * speed * deltaTime;
    if (keys[SDL_SCANCODE_A]) camera_position -= right   * speed * deltaTime;
    if (keys[SDL_SCANCODE_D]) camera_position += right   * speed * deltaTime;

    // Optional: keep target synced in front of camera
    camera_target = camera_position + forward;
}

glm::mat4 RenderManager::rotate_matrix() {
    // Optional: animate the camera around the origin
    float time = SDL_GetTicks() / 1000.0f;

    glm::mat4 model = glm::rotate(glm::mat4(1.0f), time * 5.0f, glm::vec3(0, 1, 0));
    glm::mat4 view  = glm::lookAt(camera_position, camera_target, camera_up);
    glm::mat4 proj  = glm::perspective(glm::radians(70.0f), width / (float)height, 0.1f, 100.0f);

    return proj * view * model;
}


// Load shader from file
SDL_GPUShader* RenderManager::load_shader(const std::string& path, const SDL_GPUShaderStage stage, const std::string& entry) const {
    SDL_IOStream* file = SDL_IOFromFile(path.c_str(), "r");
    if (!file) {
        SDL_LogError(SDL_LOG_CATEGORY_RENDER, "Failed to open shader: %s", path.c_str());
        return nullptr;
    }

    const Sint64 size = SDL_GetIOSize(file);
    if (size <= 0) {
        SDL_LogError(SDL_LOG_CATEGORY_RENDER, "Shader file has invalid size: %s", path.c_str());
        SDL_CloseIO(file);
        return nullptr;
    }

    std::string source(size, '\0');
    Sint64 read = SDL_ReadIO(file, source.data(), size);
    SDL_CloseIO(file);

    if (read != size) {
        SDL_LogError(SDL_LOG_CATEGORY_RENDER, "Failed to read entire shader file: %s", path.c_str());
        return nullptr;
    }

    SDL_GPUShaderCreateInfo info = {
        .code_size = static_cast<size_t>(size),
        .code = reinterpret_cast<const Uint8*>(source.data()),
        .entrypoint = entry.c_str(),
        .format = SDL_GPU_SHADERFORMAT_MSL,
        .stage = stage,
        .num_samplers = 0,
        .num_storage_textures = 0,
        .num_storage_buffers = 0,
        .num_uniform_buffers = 2,
        .props = 0
    };

    SDL_GPUShader* shader = SDL_CreateGPUShader(device, &info);
    if (!shader) {
        SDL_LogError(SDL_LOG_CATEGORY_RENDER, "Shader compilation failed (%s): %s", entry.c_str(), SDL_GetError());
    }
    return shader;
}
