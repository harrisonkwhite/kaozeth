#include "game.h"

#include <zc/debug.h>

bool GameInit(const zf::s_game_init_context& zf_context) {
    const auto game = static_cast<s_game*>(zf_context.dev_mem);

    if (!game->test_tex.LoadFromRaw(zf::s_str_view::FromRawTerminated("assets/test.png"), zf_context.gfx_res_arena, zf_context.temp_mem_arena)) {
        return false;
    }

    return true;
}

zf::e_game_tick_result GameTick(const zf::s_game_tick_context& zf_context) {
    const auto game = static_cast<s_game*>(zf_context.dev_mem);

    if (!game->world.Tick(zf_context)) {
        return zf::ek_game_tick_result_error;
    }

    return zf::ek_game_tick_result_normal;
}

bool GameRender(const zf::s_game_render_context& zf_context) {
    const auto game = static_cast<const s_game*>(zf_context.dev_mem);

    zf_context.renderer.DrawTexture(game->test_tex, {64.0f, 64.0f});
    zf_context.renderer.DrawRect({0, 0, 32, 32}, zf::colors::g_green);

    return true;
}

void GameCleanup(void* const dev_mem) {
    const auto game = static_cast<s_game*>(dev_mem);
}
