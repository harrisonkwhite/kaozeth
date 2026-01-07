#include "world.h"

namespace world {
    void init(t_world *const world) {
    }

    void tick(t_world *const world, const zf::game::t_tick_func_context &zf_context) {
        player_tick(world, zf_context);
    }

    void render(t_world *const world, zf::rendering::t_frame_context *const frame_context) {
        player_render(&world->player, frame_context);
    }
}
