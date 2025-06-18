#include <stdlib.h>
#include <zfw_game.h>

typedef struct {
    int a;
} s_game;

static bool InitGame(const s_game_init_func_data* const func_data) {
    s_game* const game = func_data->user_mem;
    return true;
}

static bool GameTick(const s_game_tick_func_data* const func_data) {
    s_game* const game = func_data->user_mem;
    return true;
}

static bool RenderGame(const s_game_render_func_data* const func_data) {
    s_game* const game = func_data->user_mem;
    return true;
}

static void CleanGame(void* const user_mem) {
}

int main() {
    const s_game_info game_info = {
        .user_mem_size = sizeof(s_game),
        .user_mem_alignment = alignof(s_game),

        .window_init_size = {1280, 720},
        .window_title = "Terraria Clone",
        .window_flags = ek_window_flag_hide_cursor | ek_window_flag_resizable,

        .init_func = InitGame,
        .tick_func = GameTick,
        .render_func = RenderGame,
        .clean_func = CleanGame,
    };

    return RunGame(&game_info) ? EXIT_SUCCESS : EXIT_FAILURE;
}
