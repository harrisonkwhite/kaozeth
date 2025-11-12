#include "game.h"

zf::t_b8 GameInit(const zf::s_game_init_context& zf_context) {
    const auto game = static_cast<s_game*>(zf_context.dev_mem);
    return true;
}

zf::e_game_tick_result GameTick(const zf::s_game_tick_context& zf_context) {
    const auto game = static_cast<s_game*>(zf_context.dev_mem);

    if (!game->world.Tick(zf_context)) {
        return zf::ek_game_tick_result_error;
    }

    return zf::ek_game_tick_result_normal;
}

zf::t_b8 GameRender(const zf::s_game_render_context& zf_context) {
    const auto game = static_cast<const s_game*>(zf_context.dev_mem);

    game->world.Render(zf_context);

    // Render mouse.
    const zf::s_rect<zf::t_f32> mouse_rect(zf::c_window::GetMousePos<zf::t_f32>(), {g_mouse_size, g_mouse_size}, {0.5f, 0.5f});
    zf_context.renderer.DrawRect(mouse_rect, zf::colors::g_red);

    return true;
}

void GameCleanup(void* const dev_mem) {
    const auto game = static_cast<s_game*>(dev_mem);
}
