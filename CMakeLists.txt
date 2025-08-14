cmake_minimum_required(VERSION 3.28)
project(terraria_clone C)

set(CMAKE_C_STANDARD 11)

add_subdirectory(zeta_framework)

add_executable(terraria_clone
    src/entry.c

    src/game.c
    src/title_screen.c
    src/world.c
    src/world_ui.c
    src/world_gen.c
    src/player.c
    src/npcs.c
    src/projectiles.c
    src/items.c
    src/inventory.c
    src/tilemap.c
    src/game.h

    src/lighting.c
    src/lighting.h

    src/particles.c
    src/particles.h
)

target_link_libraries(terraria_clone PRIVATE zfw_core)

set_target_properties(terraria_clone PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
)
