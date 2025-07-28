#include "game.h"

int main() {
    const zfw_s_game_info game_info = {
        .user_mem_size = sizeof(s_game),
        .user_mem_alignment = ALIGN_OF(s_game),

        .window_init_size = {1280, 720},
        .window_title = GAME_TITLE,
        .window_flags = zfw_ek_window_flags_hide_cursor | zfw_ek_window_flags_resizable,

        .init_func = InitGame,
        .tick_func = GameTick,
        .render_func = RenderGame,
        .clean_func = CleanGame,
    };

    return ZFW_RunGame(&game_info) ? EXIT_SUCCESS : EXIT_FAILURE;
}
