#pragma once

#include <zf/game.h>
#include "world/world.h"

const auto g_game_title = zf::s_str_view::FromRawTerminated("Kaōzeth"); // Other Ideas: "Behold a Pale Horse", "Iron Gospel"

enum e_texture {
    ek_texture_player,
    ek_texture_enemies,
    eks_texture_cnt
};

struct s_game {
    s_world world;
    zf::s_texture test_tex;
    //zf::s_static_array<zf::s_texture, eks_texture_cnt> textures;
};

[[nodiscard]] bool GameInit(const zf::s_game_init_context& zf_context);
[[nodiscard]] zf::e_game_tick_result GameTick(const zf::s_game_tick_context& zf_context);
[[nodiscard]] bool GameRender(const zf::s_game_render_context& zf_context);
void GameCleanup(void* const dev_mem);
