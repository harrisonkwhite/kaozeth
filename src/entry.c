#include <stdlib.h>
#include "game.h"

int main() {
    const zfw_s_game_info game_info = {
        .window_init_size = {1280, 720},
        .window_title = GAME_TITLE,
        .window_flags = zfw_ek_window_flags_hide_cursor | zfw_ek_window_flags_resizable,

        .dev_mem_size = sizeof(s_game),
        .dev_mem_alignment = ALIGN_OF(s_game),

        .init_func = InitGame,
        .tick_func = GameTick,
        .render_func = RenderGame,
        .clean_func = CleanGame
    };

    return ZFW_RunGame(&game_info) ? EXIT_SUCCESS : EXIT_FAILURE;
}
