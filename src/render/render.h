#ifndef RENDERER_H
#define RENDERER_H

#include <vector>
#include <SDL2/SDL.h>

struct RenderState {
    std::vector<SDL_Rect> item_rects;
    std::vector<SDL_Rect> crafting_rects;
    SDL_Rect player_rect{};
};

class RenderManager {
    public:
        explicit RenderManager(SDL_Window& window);
        ~RenderManager();

        void clear() const;
        void present() const;
        void draw_rect(const SDL_Rect& rect, SDL_Color color) const;

        void render_state(const RenderState& state);

    private:
        SDL_Renderer* renderer = nullptr;
    };

#endif // RENDERER_H
