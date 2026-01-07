#include "game.h"

int main() {
    const zf::game::t_config config = {
        .init_func = game_init,
        .deinit_func = game_deinit,
        .tick_func = game_tick,
        .render_func = game_render,

        .user_mem_size = ZF_SIZE_OF(t_game),
        .user_mem_alignment = ZF_ALIGN_OF(t_game),
    };

    zf::game::run(config);
}
