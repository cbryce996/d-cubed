#include "render.h"

RenderManager::RenderManager(SDL_Window& window) {
    renderer = SDL_CreateRenderer(&window, -1, SDL_RENDERER_ACCELERATED);
}

RenderManager::~RenderManager() {
    if (renderer) {
        SDL_DestroyRenderer(renderer);
        renderer = nullptr;
    }
}

void RenderManager::clear() const {
    SDL_SetRenderDrawColor(renderer, 20, 20, 20, 255);
    SDL_RenderClear(renderer);
}

void RenderManager::present() const {
    SDL_RenderPresent(renderer);
}

void RenderManager::draw_rect(const SDL_Rect& rect, SDL_Color color) const {
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    SDL_RenderFillRect(renderer, &rect);
}

void RenderManager::render_state(const RenderState& state) {
    clear();

    for (const auto& rect : state.item_rects) {
        draw_rect(rect, SDL_Color{0, 100, 255, 255});
    }

    for (const auto& rect : state.crafting_rects) {
        draw_rect(rect, SDL_Color{180, 0, 180, 255});
    }

    draw_rect(state.player_rect, SDL_Color{220, 50, 50, 255});

    present();
}