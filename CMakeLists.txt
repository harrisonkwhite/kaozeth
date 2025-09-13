cmake_minimum_required(VERSION 3.28)

project(kaozeth CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(zeta_framework)

add_executable(kaozeth
    src/entry.cpp
    src/game.cpp

    src/game.h
)

target_link_libraries(kaozeth PRIVATE zfw_core)

set_target_properties(kaozeth PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
)

add_custom_target(pack_assets ALL
    COMMAND $<TARGET_FILE:zfw_asset_packer>
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Packing assets..."
    VERBATIM
)

add_dependencies(kaozeth pack_assets)
