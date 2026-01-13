#include "game.h"

int main() {
    const zgl::game::t_config config = {
        .init_func = game_init,
        .deinit_func = game_deinit,
        .tick_func = game_tick,
        .render_func = game_render,

        .user_mem_size = ZCL_SIZE_OF(t_game),
        .user_mem_alignment = ZCL_ALIGN_OF(t_game),
    };

    zgl::game::run(config);
}
