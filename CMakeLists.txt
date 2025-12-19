cmake_minimum_required(VERSION 3.28)

project(kaozeth LANGUAGES C CXX)

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

add_custom_command(TARGET kaozeth POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E rm -rf "${CMAKE_SOURCE_DIR}/assets"
    COMMAND $<TARGET_FILE:zf_asset_packer> zf_asset_packing_instrs.json
    COMMAND ${CMAKE_COMMAND} -E rm -rf "$<TARGET_FILE_DIR:kaozeth>/assets"
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_SOURCE_DIR}/assets" "$<TARGET_FILE_DIR:kaozeth>/assets"
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Packing assets..."
    VERBATIM
)
