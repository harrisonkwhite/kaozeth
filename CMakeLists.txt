cmake_minimum_required(VERSION 3.28)
project(terraria_clone C)
add_subdirectory(zeta_framework)

add_executable(terraria_clone
    src/game.c
    src/world.c
    src/items.c
    src/inventory.c
    src/player.c
    src/npcs.c
    src/tilemap.c
)

target_include_directories(terraria_clone PRIVATE zeta_framework/include)
target_link_libraries(terraria_clone PRIVATE zeta_framework m)
