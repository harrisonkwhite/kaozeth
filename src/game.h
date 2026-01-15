#pragma once

#include "pch.h"
#include "assets.h"
#include "title_screen.h"
#include "world.h"

enum t_game_state : zcl::t_i32 {
    ek_game_state_none,
    ek_game_state_title_screen,
    ek_game_state_world
};

struct t_game {
    t_assets assets;

    zgl::t_sound_type_group *snd_type_group;
    zgl::t_sound_type *snd_type;

    t_game_state state;

    union {
        t_title_screen title_screen;
        t_world world;
    } state_data;
};

void GameInit(const zgl::t_game_init_func_context &zf_context);
void GameDeinit(const zgl::t_game_deinit_func_context &zf_context);
void GameTick(const zgl::t_game_tick_func_context &zf_context);
void GameRender(const zgl::t_game_render_func_context &zf_context);
