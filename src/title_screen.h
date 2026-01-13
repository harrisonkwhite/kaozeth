#pragma once

// ============================================================
// @section: Types and Constants

struct t_title_screen {
};

enum t_title_screen_tick_request {
    ek_title_screen_tick_request_none,
    ek_title_screen_tick_request_go_to_world
};

// ============================================================


// ============================================================
// @section: Functions

t_title_screen title_screen_init();
void title_screen_deinit(t_title_screen *const ts);
[[nodiscard]] t_title_screen_tick_request title_screen_tick(t_title_screen *const ts, const zgl::game::t_tick_func_context &zf_context);
void title_screen_render(const t_title_screen *const ts, zgl::gfx::t_frame_context *const frame_context, zcl::t_arena *const temp_arena);

// ============================================================
