#pragma once

struct t_player {
    zf::math::t_v2 pos;
    zf::math::t_v2 vel;
    zf::t_f32 rot;
};

constexpr zf::t_i32 g_enemy_limit = 1024;

struct t_enemy {
    zf::math::t_v2 pos;
    zf::math::t_v2 vel;
};

struct t_world {
    t_player player;

    zf::t_static_array<t_enemy, g_enemy_limit> enemies;
    zf::mem::t_static_bitset<g_enemy_limit> enemy_activity;
};

void world_init(t_world *const world);
void world_tick(t_world *const world);
void world_render(t_world *const world);

void world_player_init(t_world *const world);
void world_player_tick(t_world *const world);
void world_player_render(t_world *const world, zf::rendering::t_frame_context *const frame_context);
zf::math::t_rect_f world_player_get_collider(const zf::math::t_v2 player_pos);

void world_enemy_spawn(t_world *const world, const zf::math::t_v2 pos);
void world_enemies_tick(t_world *const world);
void world_enemies_render(t_world *const world, zf::rendering::t_frame_context *const frame_context);
