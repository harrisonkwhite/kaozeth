#include "title_screen.h"

#include "game_consts.h"
#include "assets.h"

t_title_screen title_screen_init() {
    return {};
}

void title_screen_deinit(t_title_screen *const ts) {
}

t_title_screen_tick_request title_screen_tick(t_title_screen *const ts, const zgl::game::t_tick_func_context &zf_context) {
    if (zgl::input::key_check_pressed(zf_context.input_state, zgl::input::ek_key_code_enter)) {
        return ek_title_screen_tick_request_go_to_world;
    }

    return ek_title_screen_tick_request_none;
}

void title_screen_render(const t_title_screen *const ts, zgl::gfx::t_frame_context *const frame_context, zcl::t_arena *const temp_arena) {
    zgl::gfx::frame_pass_begin(frame_context, zgl::platform::window_get_framebuffer_size_cache(), zcl::matrix_create_identity());

    const zcl::t_v2 fb_size = zcl::v2_i_to_f(zgl::platform::window_get_framebuffer_size_cache());

    zgl::gfx::frame_submit_str(frame_context, g_game_title, *assets::get_font(assets::ek_font_id_eb_garamond_256), zcl::v2_calc_compwise_prod(fb_size, {0.5f, 0.425f}), temp_arena, zgl::gfx::k_alignment_center);
    zgl::gfx::frame_submit_str(frame_context, ZCL_STR_LITERAL("Press Enter to start"), *assets::get_font(assets::ek_font_id_eb_garamond_64), zcl::v2_calc_compwise_prod(fb_size, {0.5f, 0.625f}), temp_arena, zgl::gfx::k_alignment_center);

    zgl::gfx::frame_pass_end(frame_context);
}
