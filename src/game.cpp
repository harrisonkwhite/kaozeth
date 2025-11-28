#include "game.h"

zf::t_b8 GameInit(const zf::s_game_init_context& zf_context) {
    const auto game = static_cast<s_game*>(zf_context.dev_mem);

    if (!InitTitleScreen(game->state_data.ts)) {
        return false;
    }

    if (!zf::gfx::LoadFontAssetFromPacked("assets/fonts/eb_garamond_96.zfdat", *zf_context.mem_arena, *zf_context.gfx_res_arena, *zf_context.temp_mem_arena, game->font)) {
        return false;
    }

    return true;
}

zf::e_game_tick_result GameTick(const zf::s_game_tick_context& zf_context) {
    const auto game = static_cast<s_game*>(zf_context.dev_mem);

    if (game->state == ek_game_state_title_screen) {
        const auto ts_tick_res = TitleScreenTick(game->state_data.ts);

        switch (ts_tick_res) {
        case ek_title_screen_tick_result_success:
            break;

        case ek_title_screen_tick_result_failure:
            return zf::ek_game_tick_result_error;

        case ek_title_screen_tick_result_go_to_world:
            game->state = ek_game_state_world;

            if (!InitWorld(game->state_data.world)) {
                return zf::ek_game_tick_result_error;
            }

            break;
        }
    } else {
        if (!WorldTick(game->state_data.world, zf_context)) {
            return zf::ek_game_tick_result_error;
        }
    }

    return zf::ek_game_tick_result_normal;
}

zf::t_b8 GameRender(const zf::s_game_render_context& zf_context) {
    const auto game = static_cast<const s_game*>(zf_context.dev_mem);

    switch (game->state) {
    case ek_game_state_title_screen:
        RenderTitleScreen(game->state_data.ts, zf_context);
        break;

    case ek_game_state_world:
        RenderWorld(game->state_data.world, zf_context);
        break;
    }

    const auto yeah = static_cast<zf::s_v2<zf::t_f32>>(zf::GetWindowSize());

    if (!zf::DrawStr(*zf_context.rendering_context, "Český Kŕdeľ žĺtých\nhúsí útočí\nÀÁÂÃÄÅÆÇÈÉÊËÌÍÎÏÐÑÒ\nÓÔÕÖØÙÚÛÜÝÞß\n\nna Ďábla.", game->font, yeah / 2.0f, {0.5f, 0.5f}, *zf_context.temp_mem_arena)) {
        return false;
    }

    // Render mouse.
#if 0
    const zf::s_rect<zf::t_f32> mouse_rect(zf::GetMousePos(), {g_mouse_size, g_mouse_size}, {0.5f, 0.5f});
    zf_context.renderer.DrawRect(mouse_rect, zf::colors::g_red);
#endif

    return true;
}

void GameCleanup(void* const dev_mem) {
    const auto game = static_cast<s_game*>(dev_mem);
}
