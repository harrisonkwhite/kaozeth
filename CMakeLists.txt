cmake_minimum_required(VERSION 3.28)
project(terraria_clone C)
add_subdirectory(zeta_framework)

add_executable(terraria_clone
    src/game.c
    src/title_screen.c
    src/world.c
    src/world_gen.c
    src/items.c
    src/inventory.c
    src/player.c
    src/npcs.c
    src/tiles.c
    src/projectiles.c

    src/game.h
)

target_include_directories(terraria_clone PRIVATE zeta_framework/include)
target_link_libraries(terraria_clone PRIVATE zeta_framework)

set_target_properties(terraria_clone PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
)
