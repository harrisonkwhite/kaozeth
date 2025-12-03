cmake_minimum_required(VERSION 3.28)

project(kaozeth LANGUAGES C CXX)

if (MSVC)
    add_compile_options(/W3 /permissive- /utf-8)
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_subdirectory(zeta_framework)

add_executable(kaozeth
    src/entry.cpp
    src/game.cpp
    src/title_screen.cpp
    src/world/world.cpp
    src/world/player.cpp

    src/game.h
    src/title_screen.h
    src/world/world.h

    zf_asset_packing_instrs.json
)

target_precompile_headers(kaozeth PRIVATE src/pch.h)

if (MSVC)
    if (CMAKE_BUILD_TYPE STREQUAL "Debug")
        target_link_options(kaozeth PRIVATE "/SUBSYSTEM:CONSOLE")
    else()
        target_link_options(kaozeth PRIVATE
            "/SUBSYSTEM:WINDOWS"
            "/ENTRY:mainCRTStartup"
        )
    endif()
endif()

target_link_libraries(kaozeth PRIVATE zf)

target_compile_definitions(kaozeth PRIVATE
    _CRT_SECURE_NO_WARNINGS
)

if (MSVC)
    target_compile_options(kaozeth PRIVATE
        $<$<CONFIG:Release>:/Zi /Oy->
    )
    target_link_options(kaozeth PRIVATE
        $<$<CONFIG:Release>:/DEBUG>
    )
endif()

add_custom_target(pack_assets ALL
    COMMAND $<TARGET_FILE:zf_asset_packer> zf_asset_packing_instrs.json
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Packing assets..."
    VERBATIM
)

set_target_properties(pack_assets PROPERTIES FOLDER "Build Tools")

add_custom_target(copy_assets ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/assets"
        "${CMAKE_BINARY_DIR}/assets"
    COMMENT "Copying assets..."
)

set_target_properties(copy_assets PROPERTIES FOLDER "Build Tools")

add_dependencies(copy_assets pack_assets)
add_dependencies(kaozeth copy_assets)

add_custom_command(
    TARGET kaozeth POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_BINARY_DIR}/compile_commands.json"
        "${CMAKE_SOURCE_DIR}/compile_commands.json"
    COMMENT "Copying compile commands..."
    VERBATIM
)
