cmake_minimum_required(VERSION 3.21)
project(game_project)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_OSX_ARCHITECTURES arm64)

# === Game Library ===
set(SRC_FILES
    src/main.cpp
    src/engine/engine.cpp
    src/engine/runtime/tasks/tasks.cpp
    src/engine/inputs/input.cpp
    src/engine/scene/object/demos/instancing.cpp
    src/engine/scene/object/demos/wave.cpp
    src/engine/scene/object/object.cpp
    src/engine/scene/scene.cpp
    src/engine/runtime/schedule/schedule.cpp
    src/engine/runtime/runtime.cpp
    src/engine/render/render.cpp
    src/engine/render/graph/graph.cpp
    src/engine/render/pipelines/pipeline.cpp
    src/engine/render/buffers/buffer.cpp
    src/engine/cameras/camera.cpp
    src/engine/render/shaders/shader.cpp
    src/engine/assets/asset.cpp
    src/engine/assets/loader.cpp
)

add_library(game_lib STATIC ${SRC_FILES})

add_definitions(-DSHADERS_DIR="${CMAKE_SOURCE_DIR}/assets/shaders/")

target_include_directories(game_lib PUBLIC
    src
    src/engine
        src/engine/scene
    external/tinyobjloader
)

find_package(SDL3 CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(nlohmann_json REQUIRED)

target_link_libraries(game_lib PUBLIC
    SDL3::SDL3
    glm::glm
    nlohmann_json::nlohmann_json
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
    tests/engine/test_main.cpp
    tests/engine/test_assets.cpp
    tests/engine/test_cameras.cpp
    tests/engine/test_tasks.cpp
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
