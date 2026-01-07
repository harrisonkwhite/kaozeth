cmake_minimum_required(VERSION 3.28)

project(kaozeth LANGUAGES C CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_subdirectory(zeta_framework)

add_executable(kaozeth
    src/entry.cpp
    src/game.cpp
    src/assets.cpp
    src/world_main.cpp
    src/world_player.cpp
    src/world_enemies.cpp

    src/pch.h
    src/game.h
    src/assets.h
    src/world.h
)

target_precompile_headers(kaozeth PRIVATE src/pch.h)
target_compile_features(kaozeth PRIVATE cxx_std_20)
target_link_libraries(kaozeth PRIVATE zf_game_lib)

set_target_properties(kaozeth PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:kaozeth>"
)

add_custom_target(pack_assets
    COMMAND ${CMAKE_COMMAND} -E rm -rf "${CMAKE_CURRENT_SOURCE_DIR}/assets"
    COMMAND zf_asset_packer "${CMAKE_CURRENT_SOURCE_DIR}/zf_asset_packing_instrs.json"
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_CURRENT_SOURCE_DIR}/assets" "$<TARGET_FILE_DIR:kaozeth>/assets"
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    COMMENT "Packing assets..."
    VERBATIM
)

add_dependencies(kaozeth pack_assets)
