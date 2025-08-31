cmake_minimum_required(VERSION 3.21)
project(game_project)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_OSX_ARCHITECTURES arm64)

# === SDL3 (with GPU support) ===
add_subdirectory(external/SDL EXCLUDE_FROM_ALL)
include_directories(external/SDL/include)

# === Game Sources ===
set(SRC_FILES
        src/main.cpp
        src/engine/engine.cpp
        src/engine/tasks.cpp
        src/engine/inputs.cpp
        src/game/game.cpp
        src/game/player.cpp
        src/game/update.cpp
        src/core/items/types.cpp
        src/core/items/items.cpp
        src/render/render.cpp
        src/render/graph.cpp
        src/render/mesh.cpp
        src/render/pipeline.cpp
        src/render/buffer.cpp
        src/render/camera.cpp
        src/render/shader.cpp
        src/game/entity.cpp
        src/render/drawable.h
        src/render/material.h
)

add_executable(game ${SRC_FILES})

target_include_directories(game PRIVATE
        src
        src/core/items
        src/core/buildings
        src/core/world
        src/engine
        src/game
        src/render
        external/glm
)

# === Link SDL3 with GPU support ===
target_link_libraries(game
        SDL3::SDL3
)
