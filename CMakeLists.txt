cmake_minimum_required(VERSION 3.28)

project(kaozeth LANGUAGES C CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_subdirectory(zeta_framework)

add_executable(kaozeth
    src/entry.cpp
    src/game.cpp

    src/pch.h
    src/game.h
)

target_precompile_headers(kaozeth PRIVATE src/pch.h)
target_compile_features(kaozeth PRIVATE cxx_std_20)
target_link_libraries(kaozeth PRIVATE zf_game_lib)

set_target_properties(kaozeth PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:kaozeth>"
)

add_custom_target(pack_assets ALL
    COMMAND $<TARGET_FILE:zf_asset_packer> zf_asset_packing_instrs.json
    COMMAND ${CMAKE_COMMAND} -E rm -rf "$<TARGET_FILE_DIR:kaozeth>/assets"
    COMMAND ${CMAKE_COMMAND} -E copy_directory assets "$<TARGET_FILE_DIR:kaozeth>/assets"
    COMMAND ${CMAKE_COMMAND} -E rm -rf assets
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Packing assets..."
    VERBATIM
)

add_dependencies(kaozeth pack_assets)
