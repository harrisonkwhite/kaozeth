#pragma once

namespace world {
    // ============================================================
    // @section: Types and Constants

    constexpr zcl::t_f32 k_camera_scale = 2.0f;

    struct t_player {
        zcl::math::t_v2 pos;
        zcl::math::t_v2 vel;
        zcl::t_f32 rot;
    };

    constexpr zcl::t_i32 k_enemy_limit = 1024;

    struct t_enemy {
        zcl::math::t_v2 pos;
        zcl::math::t_v2 vel;
        zcl::t_f32 rot;
    };

    struct t_world {
        zgl::gfx::t_resource_group rendering_resource_group;

        zgl::gfx::t_resource *texture_target_all;

        zcl::math::t_v2 camera_pos;

        t_player player;

        zcl::t_static_array<t_enemy, k_enemy_limit> enemies;
        zcl::mem::t_static_bitset<k_enemy_limit> enemy_activity;
    };

    // ============================================================


    // ============================================================
    // @section: Functions

    t_world create(zcl::mem::t_arena *const arena);
    void destroy(t_world *const world);
    void tick(t_world *const world, const zgl::game::t_tick_func_context &zf_context);
    void render(const t_world *const world, zgl::gfx::t_frame_context *const frame_context, zcl::mem::t_arena *const temp_arena);

    inline zcl::math::t_v2 camera_get_size() {
        return zcl::math::v2_i_to_f(zgl::platform::window_get_framebuffer_size_cache()) / k_camera_scale;
    }

    inline zcl::math::t_v2 world_to_screen_pos(const zcl::math::t_v2 pos, const zcl::math::t_v2 camera_pos) {
        return (pos - camera_pos) * k_camera_scale;
    }

    inline zcl::math::t_v2 screen_to_world_pos(const zcl::math::t_v2 pos, const zcl::math::t_v2 camera_pos) {
        return (pos / k_camera_scale) + camera_pos;
    }

    void player_init(t_world *const world);
    void player_tick(t_world *const world, const zgl::game::t_tick_func_context &zf_context);
    void player_render(const t_player *const player, zgl::gfx::t_frame_context *const frame_context);
    zcl::math::t_rect_f player_get_collider(const zcl::math::t_v2 player_pos);

    void enemy_spawn(t_world *const world, const zcl::math::t_v2 pos);
    void enemies_tick(t_world *const world);
    void enemies_render(const t_world *const world, zgl::gfx::t_frame_context *const frame_context);

    // ============================================================
}
