#pragma once

#include <zf.h>
#include "title_screen.h"
#include "world/world.h"

const zf::s_str_view g_game_title = "Kaōzeth"; // Other Ideas: "Behold a Pale Horse", "Iron Gospel"

const zf::t_f32 g_mouse_size = 4.0f;

enum e_texture {
    ek_texture_player,
    ek_texture_enemies,
    eks_texture_cnt
};

enum class ec_game_state {
    title_screen,
    world
};

struct s_game {
    zf::s_static_array<zf::s_texture, eks_texture_cnt> textures;

    ec_game_state state = ec_game_state::title_screen;
    s_title_screen ts;
    s_world world;
};

[[nodiscard]] zf::t_b8 GameInit(const zf::s_game_init_context& zf_context);
[[nodiscard]] zf::e_game_tick_result GameTick(const zf::s_game_tick_context& zf_context);
[[nodiscard]] zf::t_b8 GameRender(const zf::s_game_render_context& zf_context);
void GameCleanup(void* const dev_mem);
