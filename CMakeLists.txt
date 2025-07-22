cmake_minimum_required(VERSION 3.21)
project(game_project)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_OSX_ARCHITECTURES arm64)

# === Dependencies ===

# SDL2 (via pkg-config)
find_package(PkgConfig REQUIRED)
pkg_check_modules(SDL2 REQUIRED sdl2)
include_directories(${SDL2_INCLUDE_DIRS})
link_directories(${SDL2_LIBRARY_DIRS})
add_definitions(${SDL2_CFLAGS_OTHER})

# Boost
find_package(Boost REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})

# === Common Include Paths ===
include_directories(
        src
        src/core/items
        src/core/buildings
        src/core/world
)

# === Main Game Executable ===
add_executable(game
        src/main.cpp
        src/core/world/world_generator.cpp
        src/core/world/world_geometry.cpp
        src/core/items/types.cpp
        src/core/items/items.cpp
        src/engine/engine.cpp
        src/game/game.cpp
        src/engine/tasks.cpp
        src/engine/inputs.cpp
        src/game/player.cpp
        src/render/render.cpp
        src/engine/update.cpp
)

target_link_libraries(game PRIVATE ${SDL2_LIBRARIES})