#pragma once

#include "pch.h"

void game_init(const zf::game::t_init_func_context &zf_context);
void game_deinit();
void game_tick(const zf::game::t_tick_func_context &zf_context);
void game_render(const zf::game::t_render_func_context &zf_context);
