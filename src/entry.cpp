#include "game.h"

int main() {
    const zf::s_game_info game_info = {
        .mem_arena_size = zf::Megabytes(80),
        .temp_mem_arena_size = zf::Megabytes(10),
        .frame_mem_arena_size = zf::Megabytes(10),

        .window_init_size = {1280, 720},
        .window_init_title = g_game_title,

        .dev_mem_size = ZF_SIZE_OF(s_game),
        .dev_mem_alignment = ZF_ALIGN_OF(s_game),

        .targ_ticks_per_sec = 60,

        .init_func = GameInit,
        .tick_func = GameTick,
        .render_func = GameRender,
        .clean_func = GameCleanup
    };

    zf::RunGame(game_info);
}
