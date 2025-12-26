add_executable(run_tests
        test_main.cpp
        test_assets.cpp
)

target_link_libraries(run_tests PRIVATE gtest_main game_lib)

target_include_directories(run_tests PRIVATE ../src)

add_test(NAME EngineTests COMMAND run_tests)
