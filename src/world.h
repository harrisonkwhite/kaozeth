#pragma once

namespace world {
    // ============================================================
    // @section: Types and Constants

    struct t_player {
        zf::math::t_v2 pos;
        zf::math::t_v2 vel;
        zf::t_f32 rot;
    };

    constexpr zf::t_i32 k_enemy_limit = 1024;

    struct t_enemy {
        zf::math::t_v2 pos;
        zf::math::t_v2 vel;
    };

    struct t_world {
        zf::math::t_v2 camera_pos;

        t_player player;

        zf::t_static_array<t_enemy, k_enemy_limit> enemies;
        zf::mem::t_static_bitset<k_enemy_limit> enemy_activity;
    };

    // ============================================================


    // ============================================================
    // @section: Functions

    void init(t_world *const world);
    void tick(t_world *const world, const zf::game::t_tick_func_context &zf_context);
    void render(t_world *const world, zf::rendering::t_frame_context *const frame_context);

    void player_init(t_world *const world);
    void player_tick(t_world *const world, const zf::game::t_tick_func_context &zf_context);
    void player_render(const t_player *const player, zf::rendering::t_frame_context *const frame_context);
    zf::math::t_rect_f player_get_collider(const zf::math::t_v2 player_pos);

    void enemy_spawn(t_world *const world, const zf::math::t_v2 pos);
    void enemies_tick(t_world *const world);
    void enemies_render(t_world *const world, zf::rendering::t_frame_context *const frame_context);

    // ============================================================
}
