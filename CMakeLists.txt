cmake_minimum_required(VERSION 3.28)

project(kaozeth LANGUAGES C CXX)

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
)

target_link_libraries(kaozeth PRIVATE zf)

set_target_properties(kaozeth PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
)

add_custom_target(pack_assets ALL
    COMMAND $<TARGET_FILE:zf_asset_packer> zf_asset_packing_instrs.json
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Packing assets..."
    VERBATIM
)

add_dependencies(kaozeth pack_assets)
