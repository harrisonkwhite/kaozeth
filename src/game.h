#pragma once

#include <zf.h>
#include "title_screen.h"
#include "world/world.h"

const zf::s_str_rdonly g_game_title = "Kaōzeth"; // Other Ideas: "Behold a Pale Horse", "Iron Gospel"

const zf::t_f32 g_mouse_size = 4.0f;

enum e_texture : zf::t_s32 {
    ek_texture_player,
    ek_texture_enemies,

    eks_texture_cnt
};

enum e_game_state : zf::t_s32 {
    ek_game_state_title_screen,
    ek_game_state_world
};

struct s_game {
    zf::gfx::s_font_asset font;
    zf::audio::t_sound_type_id snd_type_id;

    e_game_state state;

    union {
        s_title_screen ts;
        s_world world;
    } state_data;
};

[[nodiscard]] zf::t_b8 GameInit(const zf::s_game_init_context& zf_context);
[[nodiscard]] zf::e_game_tick_result GameTick(const zf::s_game_tick_context& zf_context);
[[nodiscard]] zf::t_b8 GameRender(const zf::s_game_render_context& zf_context);
void GameCleanup(void* const dev_mem);
