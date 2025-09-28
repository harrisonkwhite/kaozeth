#include "world.h"

bool InitWorld(s_world& world) {
    return true;
}

bool WorldTick(s_world& world, const zf::s_game_tick_context& zf_context) {
    PlayerTick(world, zf_context);
    return true;
}
