#include <stdlib.h>
#include "game.h"
#include <mem/zc_heaps.h>

int main() {
    /*const zf::s_game_info game_info = {
        .window_init_size = {1280, 720},
        .window_title = g_game_title,
        .window_flags = static_cast<zf::e_window_flags>(zf::ek_window_flags_hide_cursor | zf::ek_window_flags_resizable),

        .dev_mem_size = sizeof(s_game),
        .dev_mem_alignment = alignof(s_game),

        .targ_ticks_per_sec = 60,

        .init_func = GameInit,
        .tick_func = GameTick,
        .render_func = GameRender,
        .clean_func = GameCleanup
    };

    return zf::RunGame(game_info) ? EXIT_SUCCESS : EXIT_FAILURE;*/

    zf::c_mem_arena mem_arena;

    if (!mem_arena.Init(zf::Megabytes(1))) {
        return EXIT_FAILURE;
    }

    zf::c_min_heap<int> mh;

    if (!mh.Init(mem_arena, 32)) {
        mem_arena.Clean();
        return EXIT_FAILURE;
    }

    mh.Insert(2);
    mh.Insert(4);
    mh.Insert(1);
    mh.Insert(8);
    mh.Insert(15);
    mh.Insert(0);
    ZF_LOG("Min heap top: %d", mh.GetMin());

    mh.RemoveMin();
    mh.RemoveMin();
    ZF_LOG("Min heap top after removal: %d", mh.GetMin());

    mem_arena.Clean();

    return EXIT_SUCCESS;
}
