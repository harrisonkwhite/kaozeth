#include "game.h"

zf::t_b8 GameInit(const zf::s_game_init_context& zf_context) {
    const auto game = static_cast<s_game*>(zf_context.dev_mem);

    zf::c_list<zf::t_f32> nums;

    if (!zf::MakeList(zf_context.temp_mem_arena, 8, nums)) {
        return false;
    }

    return true;
}

zf::e_game_tick_result GameTick(const zf::s_game_tick_context& zf_context) {
    const auto game = static_cast<s_game*>(zf_context.dev_mem);

    if (game->state == ec_game_state::title_screen) {
        const auto ts_tick_res = TitleScreenTick();

        switch (ts_tick_res) {
        case ec_title_screen_tick_result::success:
            break;

        case ec_title_screen_tick_result::failure:
            return zf::ek_game_tick_result_error;

        case ec_title_screen_tick_result::go_to_world:
            game->state = ec_game_state::world;
            break;
        }
    } else {
        if (!game->world.Tick(zf_context)) {
            return zf::ek_game_tick_result_error;
        }
    }

    return zf::ek_game_tick_result_normal;
}

zf::t_b8 GameRender(const zf::s_game_render_context& zf_context) {
    const auto game = static_cast<const s_game*>(zf_context.dev_mem);

    switch (game->state) {
    case ec_game_state::title_screen:
        RenderTitleScreen(game->ts, zf_context.renderer);
        break;

    case ec_game_state::world:
        game->world.Render(zf_context);
        break;
    }

    // Render mouse.
    const zf::s_rect<zf::t_f32> mouse_rect(zf::c_window::GetMousePos<zf::t_f32>(), {g_mouse_size, g_mouse_size}, {0.5f, 0.5f});
    zf_context.renderer.DrawRect(mouse_rect, zf::colors::g_red);

    return true;
}

void GameCleanup(void* const dev_mem) {
    const auto game = static_cast<s_game*>(dev_mem);
}
