#pragma once

#include "pch.h"

const zf::s_str_rdonly g_game_title = "Kaōzeth";

struct s_player {
    zf::s_v2<zf::t_f32> pos;
    zf::s_v2<zf::t_f32> vel;
    zf::t_f32 rot;
};

struct s_game {
    s_player player;
};

[[nodiscard]] zf::t_b8 GameInit(const zf::s_game_init_context& zf_context);
[[nodiscard]] zf::t_b8 GameTick(const zf::s_game_tick_context& zf_context);
[[nodiscard]] zf::t_b8 GameRender(const zf::s_game_render_context& zf_context);
void GameCleanup(void* const dev_mem);
