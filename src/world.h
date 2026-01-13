#pragma once

namespace world {
    // ============================================================
    // @section: Types and Constants

    constexpr zcl::t_f32 k_camera_scale = 2.0f;

    constexpr zcl::t_i32 k_player_flash_time_max = 10;

    struct t_player {
        zcl::t_v2 pos;
        zcl::t_v2 vel;
        zcl::t_f32 rot;
        zcl::t_i32 flash_time;
    };

    constexpr zcl::t_i32 k_enemy_limit = 1024;

    struct t_enemy {
        zcl::t_v2 pos;
        zcl::t_v2 vel;
        zcl::t_f32 rot;
    };

    struct t_world {
        zgl::gfx::t_resource_group rendering_resource_group;

        zgl::gfx::t_resource *texture_target_all;

        zcl::t_v2 camera_pos;

        t_player player;

        zcl::t_static_array<t_enemy, k_enemy_limit> enemies;
        zcl::t_static_bitset<k_enemy_limit> enemy_activity;
    };

    // ============================================================


    // ============================================================
    // @section: Functions

    t_world create(zcl::t_arena *const arena);
    void destroy(t_world *const world);
    void tick(t_world *const world, const zgl::game::t_tick_func_context &zf_context);
    void render(const t_world *const world, zgl::gfx::t_frame_context *const frame_context, zcl::t_arena *const temp_arena);

    inline zcl::t_v2 camera_get_size() {
        return zcl::v2_i_to_f(zgl::platform::window_get_framebuffer_size_cache()) / k_camera_scale;
    }

    inline zcl::t_v2 world_to_screen_pos(const zcl::t_v2 pos, const zcl::t_v2 camera_pos) {
        return (pos - camera_pos) * k_camera_scale;
    }

    inline zcl::t_v2 screen_to_world_pos(const zcl::t_v2 pos, const zcl::t_v2 camera_pos) {
        return (pos / k_camera_scale) + camera_pos;
    }

    void player_init(t_world *const world);
    void player_tick(t_world *const world, const zgl::game::t_tick_func_context &zf_context);
    void player_render(const t_player *const player, zgl::gfx::t_frame_context *const frame_context);
    zcl::t_rect_f player_get_collider(const zcl::t_v2 player_pos);

    void enemy_spawn(t_world *const world, const zcl::t_v2 pos);
    void enemies_tick(t_world *const world);
    void enemies_render(const t_world *const world, zgl::gfx::t_frame_context *const frame_context);

    // ============================================================
}
