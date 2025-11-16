#include "world.h"

static zf::t_size SpawnEnemy(s_world& world, const zf::s_v2<zf::t_f32> pos) {
    const zf::t_size index = world.enemies.TakeFirstInactiveSlot();

    if (index != -1) {
        s_enemy& enemy = world.enemies[index];
        enemy.pos = pos;
    }

    return index;
}

zf::t_b8 s_world::Init() {
    SpawnEnemy(*this, {});
    return true;
}

zf::t_b8 s_world::Tick(const zf::s_game_tick_context& zf_context) {
    PlayerTick(*this, zf_context);

    // Run enemy ticks.
    for (zf::t_size i = 0; i < enemies.Len(); i = enemies.IndexOfFirstActiveSlot(i + 1)) {
    }

    return true;
}

void s_world::Render(const zf::s_game_render_context& zf_context) const {
    RenderPlayer(player, zf_context.renderer);

    // Render enemies.
    for (zf::t_size i = 0; i < enemies.Len(); i = enemies.IndexOfFirstActiveSlot(i + 1)) {
    }
}
