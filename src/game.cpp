#include "game.h"

bool InitGame(const zf::s_game_init_context& zfw_context) {
    return true;
}

zf::e_game_tick_result GameTick(const zf::s_game_tick_context& zfw_context) {
    const auto game = static_cast<s_game*>(zfw_context.dev_mem);
    return zf::ek_game_tick_result_normal;
}

bool RenderGame(const zf::s_game_render_context& zfw_context) {
    const auto game = static_cast<const s_game*>(zfw_context.dev_mem);
    return true;
}

void CleanGame(void* const dev_mem) {
    const auto game = static_cast<s_game*>(dev_mem);
}
