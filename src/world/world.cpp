#include "world.h"

zf::t_b8 s_world::Init() {
    return true;
}

zf::t_b8 s_world::Tick(const zf::s_game_tick_context& zf_context) {
    PlayerTick(*this, zf_context);
    return true;
}

void s_world::Render(const zf::s_game_render_context& zf_context) const {
    RenderPlayer(player, zf_context.renderer);
}
