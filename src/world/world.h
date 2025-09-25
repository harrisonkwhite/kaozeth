#pragma once

#include <zf.h>

struct s_player {
    zf::s_v2 pos;
    zf::s_v2 vel;
};

struct s_world {
    s_player player;
};

bool InitWorld(s_world& world);
bool WorldTick(s_world& world);
bool RenderWorld(const s_world& world, zf::s_rendering_context& rc);

void PlayerTick(s_world& world);
