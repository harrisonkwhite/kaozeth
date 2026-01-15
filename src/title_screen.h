#pragma once

#include "pch.h"

struct t_title_screen {
};

enum t_title_screen_tick_request {
    ek_title_screen_tick_request_none,
    ek_title_screen_tick_request_go_to_world
};

struct t_assets;

t_title_screen TitleScreenCreate();
void TitleScreenDestroy(t_title_screen *const ts);
[[nodiscard]] t_title_screen_tick_request TitleScreenTick(t_title_screen *const ts, const zgl::t_game_tick_func_context &zf_context);
void TitleScreenRender(const t_title_screen *const ts, const zgl::t_frame_context frame_context, const t_assets *const assets, zcl::t_arena *const temp_arena);
