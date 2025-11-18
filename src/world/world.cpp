#include "world.h"

#if 0
static zf::t_size SpawnEnemy(s_world& world, const zf::s_v2<zf::t_f32> pos) {
    const zf::t_size index = world.enemies.TakeFirstInactiveSlot();

    if (index != -1) {
        s_enemy& enemy = world.enemies[index];
        enemy.pos = pos;
    }

    return index;
}
#endif

zf::t_b8 InitWorld(s_world& world) {
    world = {};
    //SpawnEnemy(*this, {});
    return true;
}

zf::t_b8 WorldTick(s_world& world, const zf::s_game_tick_context& zf_context) {
    PlayerTick(world, zf_context);
    return true;
}

void RenderWorld(const s_world& world, const zf::s_game_render_context& zf_context) {
    RenderPlayer(world.player, zf_context.rendering_context);
}
