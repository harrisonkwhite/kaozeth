cmake_minimum_required(VERSION 3.28)

project(kaozeth LANGUAGES C CXX)

if (MSVC)
    add_compile_options(/W3 /permissive-)
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_subdirectory(zeta_framework)

add_executable(kaozeth
    src/entry.cpp
    src/game.cpp
    src/world/world.cpp
    src/world/player.cpp

    src/game.h
    src/world/world.h

    zf_asset_packing_instrs.json
)

target_link_libraries(kaozeth PRIVATE zf)

add_custom_target(pack_assets ALL
    COMMAND $<TARGET_FILE:zf_asset_packer> zf_asset_packing_instrs.json
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Packing assets..."
    VERBATIM
)

add_custom_target(copy_assets ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/assets"
        "${CMAKE_BINARY_DIR}/assets"
    COMMENT "Copying assets..."
)

add_dependencies(copy_assets pack_assets)
add_dependencies(kaozeth copy_assets)
