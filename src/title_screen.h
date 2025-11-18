#pragma once

#include <zf.h>

enum class ec_title_screen_tick_result {
    success,
    failure,
    go_to_world
};

struct s_title_screen {
};

[[nodiscard]] zf::t_b8 InitTitleScreen(s_title_screen& ts);
ec_title_screen_tick_result TitleScreenTick(s_title_screen& ts);
void RenderTitleScreen(const s_title_screen& ts, const zf::s_game_render_context& zf_context);
