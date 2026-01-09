#include "world.h"

#include "assets.h"

namespace world {
    constexpr zcl::math::t_v2 k_enemy_origin = zgl::gfx::k_origin_center;

    void enemy_spawn(t_world *const world, const zcl::math::t_v2 pos) {
        const zcl::t_i32 enemy_index = zcl::mem::bitset_find_first_unset_bit(world->enemy_activity);

        if (enemy_index == -1) {
            ZF_FATAL();
        }

        world->enemies[enemy_index] = {
            .pos = pos,
        };

        zcl::mem::bitset_set(world->enemy_activity, enemy_index);
    }

    void enemies_tick(t_world *const world) {
        ZF_WALK_SET_BITS (world->enemy_activity, i) {
            t_enemy *const enemy = &world->enemies[i];
            enemy->pos += enemy->vel;
        }
    }

    void enemies_render(const t_world *const world, zgl::gfx::t_frame_context *const frame_context) {
        ZF_WALK_SET_BITS (world->enemy_activity, i) {
            const t_enemy *const enemy = &world->enemies[i];

            zgl::gfx::frame_submit_texture(frame_context, assets::get_texture(assets::ek_texture_id_temp), enemy->pos, assets::k_texture_temp_src_rects[assets::ek_texture_temp_src_rect_id_enemy], k_enemy_origin, enemy->rot);
        }
    }
}
