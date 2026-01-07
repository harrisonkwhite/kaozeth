#include "game.h"

static const zf::math::t_v2_i g_player_size = {32, 32};

static zf::rendering::t_resource *g_texture;
static zf::rendering::t_resource *g_texture_target;

static zf::math::t_v2 g_pos;
static zf::t_f32 g_rot;

void game_init(const zf::game::t_init_func_context &zf_context) {
    zf::platform::window_set_title(ZF_STR_LITERAL("Sadness"), zf_context.temp_arena);
    zf::platform::cursor_set_visible(false);

    g_texture = zf::rendering::texture_create_from_raw(ZF_STR_LITERAL("assets_raw/textures/player.png"), zf_context.temp_arena, zf_context.perm_rendering_resource_group);

    g_texture_target = zf::rendering::texture_create_target({1280, 720}, zf_context.perm_rendering_resource_group);

    // g_surface = zf::rendering::surface_create({100, 100}, zf_context.perm_rendering_resource_group);
    // g_surface = zf::rendering::texture_create_blank({100, 100}, zf_context.perm_rendering_resource_group);
}

void game_deinit() {
}

void game_tick(const zf::game::t_tick_func_context &zf_context) {
    g_pos = zf::math::v2_i_to_f(zf::platform::window_get_framebuffer_size_cache()) / 2.0f;
    g_rot = zf::math::calc_dir_in_rads(g_pos, zf::input::cursor_get_pos(zf_context.input_state));

    zf::rendering::texture_resize_target_if_needed(g_texture_target, zf::platform::window_get_framebuffer_size_cache());
}

void game_render(const zf::game::t_render_func_context &zf_context) {
    zf::rendering::frame_set_texture_target(zf_context.frame_context, g_texture_target);
    zf::rendering::frame_submit_rect(zf_context.frame_context, zf::math::rect_create_f32(0.0f, 0.0f, 32.0f, 32.0f), zf::gfx::g_color_pink);
    zf::rendering::frame_submit_texture(zf_context.frame_context, g_texture, g_pos, {}, {0.5f, 0.5f}, g_rot);
    zf::rendering::frame_unset_texture_target(zf_context.frame_context);

    zf::rendering::frame_submit_texture(zf_context.frame_context, g_texture_target, {}, {}, {}, {});
}
