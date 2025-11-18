#pragma once

#include <zf.h>

struct s_player {
    zf::s_v2<zf::t_f32> pos;
    zf::s_v2<zf::t_f32> vel;
};

struct s_enemy {
    zf::s_v2<zf::t_f32> pos;
    zf::s_v2<zf::t_f32> vel;
};

struct s_world {
    s_player player;
    zf::s_static_activity_array<s_enemy, 32> enemies;
};

[[nodiscard]] zf::t_b8 InitWorld(s_world& world);
[[nodiscard]] zf::t_b8 WorldTick(s_world& world, const zf::s_game_tick_context& zf_context);
void RenderWorld(const s_world& world, const zf::s_game_render_context& zf_context);

void PlayerTick(s_world &world, const zf::s_game_tick_context& zf_context);
void RenderPlayer(const s_player& player, const zf::s_rendering_context& rc);
