#pragma once

#include <zf.h>

enum class ec_title_screen_tick_result {
    success,
    failure,
    go_to_world
};

struct s_title_screen {
};

ec_title_screen_tick_result TitleScreenTick();
void RenderTitleScreen(const s_title_screen& ts, const zf::c_renderer& renderer);
