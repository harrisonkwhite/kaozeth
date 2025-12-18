cmake_minimum_required(VERSION 3.28)

project(kaozeth LANGUAGES C CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_subdirectory(zeta_framework)

add_executable(kaozeth
    src/entry.cpp
    src/game.cpp

    src/pch.h
    src/game.h

    zf_asset_packing_instrs.json
)

target_precompile_headers(kaozeth PRIVATE src/pch.h)
target_compile_features(zf_game_lib PRIVATE cxx_std_20)
target_link_libraries(kaozeth PRIVATE zf_game_lib)

add_custom_target(copy_assets ALL
    COMMAND ${CMAKE_COMMAND} -E rm -rf "${CMAKE_BINARY_DIR}/assets_raw" # Want to make sure deleted assets are not kept in the build folder.
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_SOURCE_DIR}/assets_raw" "${CMAKE_BINARY_DIR}/assets_raw"
)

add_dependencies(copy_assets kaozeth)

add_custom_target(copy_compile_commands ALL
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_BINARY_DIR}/compile_commands.json" "${CMAKE_SOURCE_DIR}/compile_commands.json"
)

add_dependencies(copy_compile_commands kaozeth)
