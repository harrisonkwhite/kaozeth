#pragma once

#include <zf.h>

struct s_player {
    zf::s_v2 pos;
    zf::s_v2 vel;
};

struct s_world {
    s_player player;
};

[[nodiscard]] bool InitWorld(s_world& world);
[[nodiscard]] bool WorldTick(s_world& world, const zf::s_game_tick_context& zf_context);

void PlayerTick(s_world &world, const zf::s_game_tick_context& zf_context);
