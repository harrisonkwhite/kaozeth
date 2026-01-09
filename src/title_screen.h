#pragma once

struct t_title_screen {
};

t_title_screen title_screen_init();
void title_screen_deinit(t_title_screen *const ts);
void title_screen_tick(t_title_screen *const ts, const zf::game::t_tick_func_context &zf_context);
void title_screen_render(const t_title_screen *const ts, zf::rendering::t_frame_context *const frame_context, zf::mem::t_arena *const temp_arena);
