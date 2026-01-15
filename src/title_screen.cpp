#include "title_screen.h"

#include "game_consts.h"
#include "assets.h"

t_title_screen TitleScreenCreate() {
    return {};
}

void TitleScreenDestroy(t_title_screen *const ts) {
}

t_title_screen_tick_request TitleScreenTick(t_title_screen *const ts, const zgl::t_game_tick_func_context &zf_context) {
    if (zgl::KeyCheckPressed(zf_context.input_state, zgl::ek_key_code_enter)) {
        return ek_title_screen_tick_request_go_to_world;
    }

    return ek_title_screen_tick_request_none;
}

void TitleScreenRender(const t_title_screen *const ts, const zgl::t_frame_context frame_context, const t_assets *const assets, zcl::t_arena *const temp_arena) {
    zgl::FramePassBegin(frame_context, zgl::FrameGetSize(frame_context), zcl::MatrixCreateIdentity());

    const zcl::t_v2 fb_size = zcl::V2IToF(zgl::FrameGetSize(frame_context));

    zgl::FrameSubmitStr(frame_context, g_game_title, assets->fonts[ek_font_id_eb_garamond_256], zcl::CalcCompwiseProd(fb_size, {0.5f, 0.425f}), temp_arena, zcl::k_origin_center);
    zgl::FrameSubmitStr(frame_context, ZCL_STR_LITERAL("Press Enter to start"), assets->fonts[ek_font_id_eb_garamond_64], zcl::CalcCompwiseProd(fb_size, {0.5f, 0.625f}), temp_arena, zcl::k_origin_center);

    zgl::FramePassEnd(frame_context);
}
