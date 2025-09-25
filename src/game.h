#pragma once

#include <zf.h>
#include "world/world.h"

const zf::c_string_view g_game_title = "Kaōzeth"; // Other Ideas: "Behold a Pale Horse", "Iron Gospel"

struct s_game {
    s_world world;
};

bool InitGame(const zf::s_game_init_context& zfw_context);
zf::e_game_tick_result GameTick(const zf::s_game_tick_context& zfw_context);
bool RenderGame(const zf::s_game_render_context& zfw_context);
void CleanGame(void* const dev_mem);
