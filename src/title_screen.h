#pragma once

#include <zf.h>

enum e_title_screen_tick_result : zf::t_s32 {
    ek_title_screen_tick_result_success,
    ek_title_screen_tick_result_failure,
    ek_title_screen_tick_result_go_to_world
};

struct s_title_screen {
};

[[nodiscard]] zf::t_b8 InitTitleScreen(s_title_screen& ts);
e_title_screen_tick_result TitleScreenTick(s_title_screen& ts);
void RenderTitleScreen(const s_title_screen& ts, const zf::s_game_render_context& zf_context);
