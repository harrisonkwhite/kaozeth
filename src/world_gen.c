#include <zfw_random.h>
#include "game.h"

#define GROUND_LEVEL_BASE (TILEMAP_HEIGHT / 3)
#define GROUND_LEVEL_OFFS_LIM 4
#define GROUND_LEVEL_TOP (GROUND_LEVEL_BASE - GROUND_LEVEL_OFFS_LIM)
#define GROUND_LEVEL_BOTTOM (GROUND_LEVEL_BASE + GROUND_LEVEL_OFFS_LIM)
static_assert(GROUND_LEVEL_TOP >= 0 && GROUND_LEVEL_BOTTOM <= TILEMAP_HEIGHT, "Tilemap ground level range goes out of bounds!");
#define GROUND_LEVEL_OFFS_VAR 0.3f
static_assert(GROUND_LEVEL_OFFS_VAR >= 0.0f && GROUND_LEVEL_OFFS_VAR <= 1.0f, "Variance is out of range!");

static void GenWorldTilemapGround(s_tilemap_core* const tm_core) {
    assert(tm_core);

    int level = GROUND_LEVEL_BASE + RandRangeI(-GROUND_LEVEL_OFFS_LIM, GROUND_LEVEL_OFFS_LIM);

    for (int tx = 0; tx < TILEMAP_WIDTH; tx++) {
        for (int ty = level; ty < TILEMAP_HEIGHT; ty++) {
            PlaceTile(tm_core, (s_vec_2d_i){tx, ty}, ek_tile_type_dirt);
        }

        if (RandPerc() < GROUND_LEVEL_OFFS_VAR) {
            if (RandPerc() < 0.5f) {
                level++;
            } else {
                level--;
            }

            level = CLAMP(level, GROUND_LEVEL_TOP, GROUND_LEVEL_BOTTOM - 1);
        }
    }
}

void GenWorld(s_world_core* const world_core) {
    assert(world_core && IS_ZERO(*world_core));

    world_core->player_hp_max = PLAYER_INIT_HP_MAX;

    GenWorldTilemapGround(&world_core->tilemap_core);
}
