#include "game.h"

#include "assets.h"

void game_init(const zf::game::t_init_func_context &zf_context) {
    const auto game = static_cast<t_game *>(zf_context.user_mem);

    zf::platform::window_set_title(g_game_title, zf_context.temp_arena);
    zf::platform::cursor_set_visible(false);

    assets::load_all(zf_context.perm_arena, zf_context.temp_arena);

    world::init(&game->world, zf_context.perm_arena);
}

void game_deinit(void *const user_mem) {
    const auto game = static_cast<t_game *>(user_mem);
    world::deinit(&game->world);
    assets::unload_all();
}

void game_tick(const zf::game::t_tick_func_context &zf_context) {
    const auto game = static_cast<t_game *>(zf_context.user_mem);

    world::tick(&game->world, zf_context);
}

void game_render(const zf::game::t_render_func_context &zf_context) {
    const auto game = static_cast<t_game *>(zf_context.user_mem);

    world::render(&game->world, zf_context.frame_context);

    zf::rendering::frame_pass_begin(zf_context.frame_context, zf::platform::window_get_framebuffer_size_cache(), zf::math::matrix_create_identity());

    zf::rendering::frame_submit_str(zf_context.frame_context, ZF_STR_LITERAL("KAŌZETH"), *assets::get_font(assets::ek_font_id_eb_garamond_128), {}, zf_context.temp_arena);

    world::render_ui(&game->world, zf_context.frame_context, zf_context.temp_arena);

    zf::rendering::frame_pass_end(zf_context.frame_context);
}
