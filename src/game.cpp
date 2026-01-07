#include "game.h"

void game_init(const zf::game::t_init_func_context &zf_context) {
    const auto game = static_cast<t_game *>(zf_context.user_mem);

    zf::platform::window_set_title(g_game_title, zf_context.temp_arena);
    zf::platform::cursor_set_visible(false);

    world_init(&game->world);
}

void game_deinit(void *const user_mem) {
    const auto game = static_cast<t_game *>(user_mem);
}

void game_tick(const zf::game::t_tick_func_context &zf_context) {
    const auto game = static_cast<t_game *>(zf_context.user_mem);

    world_tick(&game->world);
}

void game_render(const zf::game::t_render_func_context &zf_context) {
    const auto game = static_cast<t_game *>(zf_context.user_mem);

    world_render(&game->world);
}
