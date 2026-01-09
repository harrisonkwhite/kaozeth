#pragma once

#include "pch.h"
#include "title_screen.h"
#include "world.h"

enum t_game_state : zcl::t_i32 {
    ek_game_state_none,
    ek_game_state_title_screen,
    ek_game_state_world
};

struct t_game {
    t_game_state state;

    union {
        t_title_screen title_screen;
        world::t_world world;
    } state_data;
};

void game_init(const zgl::game::t_init_func_context &zf_context);
void game_deinit(void *const user_mem);
void game_tick(const zgl::game::t_tick_func_context &zf_context);
void game_render(const zgl::game::t_render_func_context &zf_context);
