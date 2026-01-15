#include "game.h"

int main() {
    const zgl::t_game_config config = {
        .init_func = GameInit,
        .deinit_func = GameDeinit,
        .tick_func = GameTick,
        .render_func = GameRender,

        .user_mem_size = ZCL_SIZE_OF(t_game),
        .user_mem_alignment = ZCL_ALIGN_OF(t_game),

        .tps_target = 60.0,
    };

    zgl::GameRun(config);
}
