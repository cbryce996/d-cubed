#ifndef RENDERER_H
#define RENDERER_H

#include <vector>
#include <string>
#include <SDL3/SDL.h>
#include <SDL3/SDL_gpu.h>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>

struct alignas(16) Uniform {
    glm::mat4 mvp;
};

struct Vertex {
    glm::vec3 position;
};

struct RenderState {
    std::vector<SDL_FRect> item_rects;
    std::vector<SDL_FRect> crafting_rects;
    SDL_FRect player_rect{};
};

class RenderManager {
public:
    RenderManager(SDL_GPUDevice* device, SDL_Window* window);
    ~RenderManager();

    void render_state(const RenderState& state);
    void update_camera(float deltaTime, const bool* keyboard_state);

private:
    // === Lifecycle ===
    void load_shaders();
    void setup_vertex_format();
    void create_pipeline();
    void create_buffers();

    glm::mat4 rotate_matrix();


    SDL_GPUShader* load_shader(const std::string& path, SDL_GPUShaderStage stage, const std::string& entry) const;

    // == Canvas ==
    Uint32 width;
    Uint32 height;

    // === Camera Context ===
    glm::vec3 camera_position = glm::vec3(0.0f, 0.5f, 2.0f);
    glm::vec3 camera_target   = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 camera_up       = glm::vec3(0.0f, 1.0f, 0.0f);

    // === GPU Context ===
    SDL_GPUDevice* device = nullptr;
    SDL_Window* window = nullptr;

    // === Shaders and Pipeline ===
    SDL_GPUShader* vertex_shader = nullptr;
    SDL_GPUShader* fragment_shader = nullptr;
    SDL_GPUGraphicsPipeline* pipeline = nullptr;

    // === Buffers ===
    SDL_GPUBuffer* vertex_buffer = nullptr;
    SDL_GPUTransferBuffer* transfer_buffer = nullptr;
    SDL_GPUBuffer* uniform_buffer = nullptr;
    SDL_GPUTransferBuffer* uniform_transfer_buffer = nullptr;


    // === Vertex Format ===
    SDL_GPUVertexAttribute attr{};
    SDL_GPUVertexBufferDescription vbo_desc{};
    SDL_GPUVertexInputState vi_state{};
};

#endif // RENDERER_H
