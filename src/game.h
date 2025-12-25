#pragma once

#include "pch.h"

constexpr zf::s_cstr_literal g_game_title = "Kaōzeth";

constexpr zf::s_v2 g_cursor_size = {4.0f, 4.0f};

struct s_player {
    zf::s_v2 pos = {};
    zf::s_v2 vel = {};
    zf::t_f32 rot = 0.0f;
};

struct s_game {
    zf::s_ptr<zf::s_gfx_resource> player_texture = nullptr;
    zf::s_ptr<zf::s_gfx_resource> enemy_texture = nullptr;

    zf::s_font font = {};

    zf::s_sound_type_arena snd_type_arena;

    struct {
        zf::s_ptr<zf::s_sound_type> explosion;
    } snd_types;

    s_player player = {};
};

void GameInit(const zf::s_game_init_context &zf_context);
void GameTick(const zf::s_game_tick_context &zf_context);
void GameRender(const zf::s_game_render_context &zf_context);
void GameCleanup();
