#include "world.h"

zf::t_b8 InitWorld(s_world& world) {
    world = {};
    return true;
}

zf::t_b8 WorldTick(s_world& world, const zf::s_game_tick_context& zf_context) {
    PlayerTick(world, zf_context);
    return true;
}

void RenderWorld(const s_world& world, const zf::s_game_render_context& zf_context) {
    RenderPlayer(world.player, *zf_context.rendering_context);
}
