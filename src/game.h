#pragma once

#include "pch.h"

void init_game(const zf::s_game_init_context &zf_context);
void deinit_game();
void run_game_tick(const zf::s_game_tick_context &zf_context);
void render_game(const zf::s_game_render_context &zf_context);
