cmake_minimum_required(VERSION 3.28)
project(terraria_clone C)

set(CMAKE_C_STANDARD 11)

add_subdirectory(zeta_framework)

add_executable(terraria_clone
    src/game.c
    src/game.h

    src/inventory.c
    src/inventory.h

    src/lighting.c
    src/lighting.h

    src/particles.c
    src/particles.h

    src/title_screen.c
    src/title_screen.h

    src/world/world.c
    src/world/world_ui.c
    src/world/world_gen.c
    src/world/player.c
    src/world/npcs.c
    src/world/projectiles.c
    src/world/tiles.c
    src/world/items.c
    src/world/world.h
)

target_include_directories(terraria_clone PRIVATE zeta_framework/include)
target_link_libraries(terraria_clone PRIVATE zeta_framework)

set_target_properties(terraria_clone PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
)
