cmake_minimum_required(VERSION 3.28)

project(kaozeth LANGUAGES C CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_subdirectory(zeta_framework)

add_executable(kaozeth
    src/entry.cpp
    src/game.cpp
    src/world_main.cpp
    src/world_player.cpp
    src/world_enemies.cpp

    src/pch.h
    src/game.h
    src/world.h
)

target_precompile_headers(kaozeth PRIVATE src/pch.h)
target_compile_features(kaozeth PRIVATE cxx_std_20)
target_link_libraries(kaozeth PRIVATE zf_game_lib)

set_target_properties(kaozeth PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:kaozeth>"
)
