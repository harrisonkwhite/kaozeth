#include "title_screen.h"

zf::t_b8 InitTitleScreen(s_title_screen& ts) {
    return true;
}

e_title_screen_tick_result TitleScreenTick(s_title_screen& ts) {
#if 0
    if (zf::window::IsKeyPressed(zf::window::ek_key_code_enter)) {
        return ek_title_screen_tick_result_go_to_world;
    }
#endif

    return ek_title_screen_tick_result_success;
}

void RenderTitleScreen(const s_title_screen& ts, const zf::s_game_render_context& zf_context) {
}
