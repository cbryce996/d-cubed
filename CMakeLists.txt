cmake_minimum_required(VERSION 3.21)
project(game_project LANGUAGES CXX)

# ------------------------------------------------------------------------------
# Global configuration
# ------------------------------------------------------------------------------
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_OSX_ARCHITECTURES arm64)

add_definitions(
        -DSHADERS_DIR="${CMAKE_SOURCE_DIR}/assets/shaders/"
)

# ------------------------------------------------------------------------------
# Coverage
# ------------------------------------------------------------------------------
option(ENABLE_COVERAGE "Enable coverage" OFF)

if(ENABLE_COVERAGE)
    message(STATUS "Coverage enabled")
    add_compile_options(-fprofile-instr-generate -fcoverage-mapping)
    add_link_options(-fprofile-instr-generate)
endif()

# ------------------------------------------------------------------------------
# Game library
# ------------------------------------------------------------------------------
set(GAME_LIB_SOURCES
        src/main.cpp
        src/engine/engine.cpp
        src/engine/runtime/tasks/tasks.cpp
        src/engine/inputs/input.cpp
        src/engine/scene/entity/demos/wave.cpp
        src/engine/scene/entity/entity.cpp
        src/engine/scene/scene.cpp
        src/engine/runtime/schedule/schedule.cpp
        src/engine/runtime/runtime.cpp
        src/engine/mesh/mesh.cpp
        src/engine/render/render.cpp
        src/engine/render/graph/graph.cpp
        src/engine/render/pass/pass.cpp
        src/engine/render/frame/frame.cpp
        src/engine/render/pipelines/pipeline.cpp
        src/engine/render/pipelines/sdl/factory.cpp
        src/engine/render/buffers/buffer.cpp
        src/engine/cameras/camera.cpp
        src/engine/render/shaders/shader.cpp
        src/engine/assets/asset.cpp
        src/engine/assets/loader.cpp
)

add_library(game_lib STATIC ${GAME_LIB_SOURCES})

target_compile_features(game_lib PUBLIC cxx_std_20)

target_include_directories(game_lib PUBLIC
        src
        src/engine
        src/engine/scene
        external/tinyobjloader
)

# ------------------------------------------------------------------------------
# Dependencies
# ------------------------------------------------------------------------------
find_package(SDL3 CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(nlohmann_json REQUIRED)

target_link_libraries(game_lib PUBLIC
        SDL3::SDL3
        glm::glm
        nlohmann_json::nlohmann_json
)

# ------------------------------------------------------------------------------
# Game executable
# ------------------------------------------------------------------------------
add_executable(game src/main.cpp)

target_compile_features(game PRIVATE cxx_std_20)

target_link_libraries(game PRIVATE
        game_lib
)

# ------------------------------------------------------------------------------
# Tests (GoogleTest via Homebrew)
# ------------------------------------------------------------------------------
find_path(GTEST_INCLUDE_DIR gtest/gtest.h PATHS /opt/homebrew/include)
find_library(GTEST_LIBRARY gtest PATHS /opt/homebrew/lib)
find_library(GTEST_MAIN_LIBRARY gtest_main PATHS /opt/homebrew/lib)

enable_testing()
include(CTest)

add_executable(engine_tests
        tests/engine/test_main.cpp
        tests/engine/test_assets.cpp
        tests/engine/test_cameras.cpp
        tests/engine/test_tasks.cpp
        tests/engine/render/test_pipelines.cpp
        tests/engine/render/test_mesh.cpp
        tests/engine/render/test_graph.cpp
        tests/engine/render/test_material.cpp
        tests/engine/render/test_pass.cpp
)

target_compile_features(engine_tests PRIVATE cxx_std_20)

target_include_directories(engine_tests PRIVATE
        src
        ${GTEST_INCLUDE_DIR}
)

target_link_libraries(engine_tests PRIVATE
        game_lib
        ${GTEST_LIBRARY}
        ${GTEST_MAIN_LIBRARY}
)

# ------------------------------------------------------------------------------
# Test helpers
# ------------------------------------------------------------------------------
add_custom_target(check
        COMMAND ${CMAKE_COMMAND} -E echo "Running unit tests..."
        COMMAND engine_tests
        DEPENDS engine_tests
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)

add_test(
        NAME EngineTests
        COMMAND engine_tests
)
