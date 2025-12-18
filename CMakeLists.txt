cmake_minimum_required(VERSION 3.28)

project(kaozeth LANGUAGES C CXX)

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
