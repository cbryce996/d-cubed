cmake_minimum_required(VERSION 3.21)
project(game_project)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_OSX_ARCHITECTURES arm64)

# === Game Library ===
set(SRC_FILES
    src/main.cpp
    src/engine/engine.cpp
        src/engine/tasks/tasks.cpp
    src/engine/inputs/input.cpp
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
    src/engine/cameras/camera.cpp
    src/render/shader.cpp
    src/game/entity.cpp
    src/engine/assets/asset.cpp
    src/engine/assets/loader.cpp
)

add_library(game_lib STATIC ${SRC_FILES})

target_include_directories(game_lib PUBLIC
    src
    src/core/items
    src/core/buildings
    src/core/world
    src/engine
    src/game
    src/render
    external/tinyobjloader
)

find_package(SDL3 CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)

target_link_libraries(game_lib PUBLIC
    SDL3::SDL3
    glm::glm
)

add_executable(game src/main.cpp)
target_link_libraries(game PRIVATE game_lib)

# === GoogleTest from Homebrew ===
find_path(GTEST_INCLUDE_DIR gtest/gtest.h PATHS /opt/homebrew/include)
find_library(GTEST_LIBRARY gtest PATHS /opt/homebrew/lib)
find_library(GTEST_MAIN_LIBRARY gtest_main PATHS /opt/homebrew/lib)

enable_testing()
include(CTest)

add_executable(engine_tests
    tests/test_main.cpp
    tests/test_assets.cpp
    tests/test_cameras.cpp
    tests/test_tasks.cpp
)

target_include_directories(engine_tests PRIVATE src ${GTEST_INCLUDE_DIR})
target_link_libraries(engine_tests PRIVATE game_lib ${GTEST_LIBRARY} ${GTEST_MAIN_LIBRARY})

add_custom_target(check
    COMMAND ${CMAKE_COMMAND} -E echo "Running unit tests..."
    COMMAND engine_tests || ${CMAKE_COMMAND} -E echo "❌ Tests failed" && exit 1
    DEPENDS engine_tests
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)

add_test(NAME EngineTests COMMAND engine_tests)
