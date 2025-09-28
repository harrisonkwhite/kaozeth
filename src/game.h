#pragma once

#include <zf.h>
#include "world/world.h"

const zf::c_string_view g_game_title = "Kaōzeth"; // Other Ideas: "Behold a Pale Horse", "Iron Gospel"

struct s_game {
    s_world world;
};

[[nodiscard]] bool InitGame(const zf::s_game_init_context& zf_context);
[[nodiscard]] zf::e_game_tick_result GameTick(const zf::s_game_tick_context& zf_context);
[[nodiscard]] bool RenderGame(const zf::s_game_render_context& zf_context);
void CleanGame(void* const dev_mem);
