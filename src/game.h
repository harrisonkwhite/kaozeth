#pragma once

#include "pch.h"
#include "world.h"

inline const zf::strs::t_str_rdonly g_game_title = ZF_STR_LITERAL("Kaōzeth");

struct t_game {
    t_world world;
};

void game_init(const zf::game::t_init_func_context &zf_context);
void game_deinit(void *const user_mem);
void game_tick(const zf::game::t_tick_func_context &zf_context);
void game_render(const zf::game::t_render_func_context &zf_context);
