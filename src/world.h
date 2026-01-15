#pragma once

constexpr zcl::t_f32 k_camera_scale = 2.0f;

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
    zgl::t_gfx_resource_group *gfx_resource_group;

    zgl::t_gfx_resource *texture_target_all;

    zcl::t_b8 paused;

    zcl::t_v2 camera_pos;

    t_player player;

    zcl::t_static_array<t_enemy, k_enemy_limit> enemies;
    zcl::t_static_bitset<k_enemy_limit> enemy_activity;
};

struct t_assets;

t_world WorldCreate(zgl::t_gfx *const gfx, const zgl::t_platform *const platform, zcl::t_arena *const arena);
void WorldDestroy(t_world *const world, zgl::t_gfx *const gfx);
void WorldTick(t_world *const world, const zgl::t_game_tick_func_context &zf_context);
void WorldRender(const t_world *const world, const zgl::t_frame_context frame_context, const t_assets *const assets, zcl::t_arena *const temp_arena);

inline zcl::t_v2 CameraGetSize(const zcl::t_v2_i screen_size) {
    return zcl::V2IToF(screen_size) / k_camera_scale;
}

inline zcl::t_v2 WorldToScreenPos(const zcl::t_v2 pos, const zcl::t_v2 camera_pos) {
    return (pos - camera_pos) * k_camera_scale;
}

inline zcl::t_v2 ScreenToWorldPos(const zcl::t_v2 pos, const zcl::t_v2 camera_pos) {
    return (pos / k_camera_scale) + camera_pos;
}

void PlayerInit(t_world *const world);
void PlayerTick(t_world *const world, const zgl::t_input_state *const input_state);
void PlayerRender(const t_player *const player, const zgl::t_frame_context frame_context, const t_assets *const assets);
zcl::t_rect_f PlayerGetCollider(const zcl::t_v2 player_pos);

void EnemySpawn(t_world *const world, const zcl::t_v2 pos);
void EnemiesTick(t_world *const world);
void EnemiesRender(const t_world *const world, const zgl::t_frame_context frame_context, const t_assets *const assets);
