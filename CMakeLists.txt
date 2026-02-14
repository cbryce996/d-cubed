cmake_minimum_required(VERSION 3.21)
project(game_project LANGUAGES CXX)

# ------------------------------------------------------------------------------
# Global configuration
# ------------------------------------------------------------------------------
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(APPLE)
    set(CMAKE_OSX_ARCHITECTURES arm64)
endif()

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
# Dependencies (provided by vcpkg)
# ------------------------------------------------------------------------------
find_package(SDL3 CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(tinyobjloader CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)

# ------------------------------------------------------------------------------
# Game library
# ------------------------------------------------------------------------------
set(GAME_LIB_SOURCES
        src/engine/engine.cpp
        src/engine/runtime/tasks/tasks.cpp
        src/engine/inputs/input.cpp
        src/engine/editor/panels/viewport/viewport.cpp
        src/engine/editor/panels/inspector/inspector.cpp
        src/engine/editor/panels/hierarchy/hierarchy.cpp
        src/engine/editor/panels/stats/stats.cpp
        src/engine/editor/panels/console/console.cpp
        src/engine/editor/panels/menu/menu.cpp
        src/engine/editor/panels/panel.cpp
        src/engine/editor/editor.cpp
        src/engine/scene/entity/components/prefabs/instancing.cpp
        src/engine/scene/entity/components/prefabs/wave.cpp
        src/engine/scene/entity/prefabs/spheres.cpp
        src/engine/scene/entity/prefabs/static.cpp
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
        src/engine/render/resources/targets/target.cpp
        src/engine/render/resources/resources.cpp
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
)

target_compile_definitions(game_lib PUBLIC
        SHADERS_DIR="${CMAKE_SOURCE_DIR}/assets/shaders/"
)

target_link_libraries(game_lib PUBLIC
        SDL3::SDL3
        glm::glm
        nlohmann_json::nlohmann_json
        tinyobjloader::tinyobjloader
        imgui::imgui
)

# If you still need the Metal frameworks on macOS for your SDL GPU path,
# link them on YOUR target(s) (safe even when imgui comes from vcpkg).
if(APPLE)
    target_link_libraries(game_lib PUBLIC
            "-framework Metal"
            "-framework QuartzCore"
            "-framework Foundation"
    )
endif()

# ------------------------------------------------------------------------------
# Game executable
# ------------------------------------------------------------------------------
add_executable(game src/main.cpp)
target_compile_features(game PRIVATE cxx_std_20)
target_link_libraries(game PRIVATE game_lib)

# ------------------------------------------------------------------------------
# Tests (GoogleTest via vcpkg)
# ------------------------------------------------------------------------------
enable_testing()
include(CTest)

find_package(GTest CONFIG REQUIRED)

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
        tests/engine/scene/test_scene.cpp
        tests/engine/scene/test_entity.cpp
)

target_compile_features(engine_tests PRIVATE cxx_std_20)

target_include_directories(engine_tests PRIVATE
        src
)

target_link_libraries(engine_tests PRIVATE
        game_lib
        GTest::gtest
        GTest::gtest_main
)

add_custom_target(check
        COMMAND ${CMAKE_COMMAND} -E echo "Running unit tests..."
        COMMAND engine_tests
        DEPENDS engine_tests
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)

add_test(NAME EngineTests COMMAND engine_tests)
