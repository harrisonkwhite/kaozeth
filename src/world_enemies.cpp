#include "world.h"

#include "assets.h"

constexpr zcl::t_v2 k_enemy_origin = zcl::k_origin_center;

void EnemySpawn(t_world *const world, const zcl::t_v2 pos) {
    const zcl::t_i32 enemy_index = zcl::BitsetFindFirstUnsetBit(world->enemy_activity);

    if (enemy_index == -1) {
        ZCL_FATAL();
    }

    world->enemies[enemy_index] = {
        .pos = pos,
    };

    zcl::BitsetSet(world->enemy_activity, enemy_index);
}

void EnemiesTick(t_world *const world) {
    ZCL_BITSET_WALK_ALL_SET (world->enemy_activity, i) {
        t_enemy *const enemy = &world->enemies[i];
        enemy->pos += enemy->vel;
    }
}

void EnemiesRender(const t_world *const world, const zgl::t_frame_context frame_context, const t_assets *const assets) {
    ZCL_BITSET_WALK_ALL_SET (world->enemy_activity, i) {
        const t_enemy *const enemy = &world->enemies[i];

        zgl::FrameSubmitTexture(frame_context, assets->textures[ek_texture_id_temp], enemy->pos, k_texture_temp_src_rects[ek_texture_temp_src_rect_id_enemy], k_enemy_origin, enemy->rot);
    }
}
