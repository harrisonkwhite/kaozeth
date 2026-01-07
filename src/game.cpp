#include "game.h"

void game_init(const zf::game::t_init_func_context &zf_context) {
    const auto game = static_cast<t_game *>(zf_context.user_mem);

    zf::mem::clear_item(game, 0);

    zf::platform::window_set_title(g_game_title, zf_context.temp_arena);
    zf::platform::cursor_set_visible(false);

    world::init(&game->world);
}

void game_deinit(void *const user_mem) {
    const auto game = static_cast<t_game *>(user_mem);
}

void game_tick(const zf::game::t_tick_func_context &zf_context) {
    const auto game = static_cast<t_game *>(zf_context.user_mem);

    world::tick(&game->world, zf_context);
}

void game_render(const zf::game::t_render_func_context &zf_context) {
    const auto game = static_cast<t_game *>(zf_context.user_mem);

    world::render(&game->world, zf_context.frame_context);
}
