#pragma once

#include <zf.h>

struct s_player {
    zf::s_v2<zf::t_f32> pos;
    zf::s_v2<zf::t_f32> vel;
};

struct s_world {
    s_player player;

    [[nodiscard]] zf::t_b8 Init();
    [[nodiscard]] zf::t_b8 Tick(const zf::s_game_tick_context& zf_context);
    void Render(const zf::s_game_render_context& zf_context) const;
};

void PlayerTick(s_world &world, const zf::s_game_tick_context& zf_context);
void RenderPlayer(const s_player& player, zf::c_renderer& renderer);
