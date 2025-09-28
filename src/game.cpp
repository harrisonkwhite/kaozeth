#include "game.h"

bool InitGame(const zf::s_game_init_context& zf_context) {
    return true;
}

zf::e_game_tick_result GameTick(const zf::s_game_tick_context& zf_context) {
    const auto game = static_cast<s_game*>(zf_context.dev_mem);

    if (!WorldTick(game->world, zf_context)) {
        return zf::ek_game_tick_result_error;
    }

    return zf::ek_game_tick_result_normal;
}

bool RenderGame(const zf::s_game_render_context& zf_context) {
    const auto game = static_cast<const s_game*>(zf_context.dev_mem);
    return true;
}

void CleanGame(void* const dev_mem) {
    const auto game = static_cast<s_game*>(dev_mem);
}
