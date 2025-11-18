#include "game.h"

zf::t_b8 GameInit(const zf::s_game_init_context& zf_context) {
    const auto game = static_cast<s_game*>(zf_context.dev_mem);

    if (!InitTitleScreen(game->ts)) {
        return false;
    }

    return true;
}

zf::e_game_tick_result GameTick(const zf::s_game_tick_context& zf_context) {
    const auto game = static_cast<s_game*>(zf_context.dev_mem);

    if (game->state == ec_game_state::title_screen) {
        const auto ts_tick_res = TitleScreenTick(game->ts);

        switch (ts_tick_res) {
        case ec_title_screen_tick_result::success:
            break;

        case ec_title_screen_tick_result::failure:
            return zf::ek_game_tick_result_error;

        case ec_title_screen_tick_result::go_to_world:
            game->state = ec_game_state::world;

            if (!InitWorld(game->world)) {
                return zf::ek_game_tick_result_error;
            }

            break;
        }
    } else {
        if (!WorldTick(game->world, zf_context)) {
            return zf::ek_game_tick_result_error;
        }
    }

    return zf::ek_game_tick_result_normal;
}

zf::t_b8 GameRender(const zf::s_game_render_context& zf_context) {
    const auto game = static_cast<const s_game*>(zf_context.dev_mem);

    switch (game->state) {
    case ec_game_state::title_screen:
        RenderTitleScreen(game->ts, zf_context);
        break;

    case ec_game_state::world:
        RenderWorld(game->world, zf_context);
        break;
    }

    // Render mouse.
#if 0
    const zf::s_rect<zf::t_f32> mouse_rect(zf::GetMousePos(), {g_mouse_size, g_mouse_size}, {0.5f, 0.5f});
    zf_context.renderer.DrawRect(mouse_rect, zf::colors::g_red);
#endif

    return true;
}

void GameCleanup(void* const dev_mem) {
    const auto game = static_cast<s_game*>(dev_mem);
}
