#pragma once

#include "pch.h"

void InitGame(const zf_s_game_init_context &zf_context);
void DeinitGame();
void RunGameTick(const zf_s_game_tick_context &zf_context);
void RenderGame(const zf_s_game_render_context &zf_context);
