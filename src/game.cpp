#include "game.h"

zf::t_b8 GameInit(const zf::s_game_init_context& zf_context) {
    const auto game = static_cast<s_game*>(zf_context.dev_mem);

    zf::s_static_array<zf::t_size, 32> nums;
    Copy(Slice(nums, 1, 4), Slice(nums, 8, 11));

    if (!InitTitleScreen(game->state_data.ts)) {
        ZF_REPORT_ERROR();
        return false;
    }

#if 0
    if (!zf::gfx::LoadTextureAssetFromPacked("assets/textures/test.zfdat", *zf_context.gfx_res_arena, *zf_context.temp_mem_arena, game->tex)) {
        ZF_REPORT_ERROR();
        return false;
    }

    if (!zf::gfx::LoadFontAssetFromPacked("assets/fonts/eb_garamond_96.zfdat", *zf_context.mem_arena, *zf_context.gfx_res_arena, *zf_context.temp_mem_arena, game->font)) {
        ZF_REPORT_ERROR();
        return false;
    }

    game->surf = zf::gfx::MakeSurface(*zf_context.gfx_res_arena, {100, 100});

    if (!zf::gfx::IsResourceHandleValid(game->surf)) {
        ZF_REPORT_ERROR();
        return false;
    }
#endif

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

#if 0
    zf::SetSurface(*zf_context.rendering_context, game->surf);
    zf::DrawClear();
    zf::DrawRect(*zf_context.rendering_context, {0, 0, 32, 32}, {1.0f, 0.0f, 0.0f, 0.5f});
    zf::UnsetSurface(*zf_context.rendering_context);

    zf::SetSurfaceShaderProg(*zf_context.rendering_context, zf_context.rendering_context->basis->surf_default_shader_prog_hdl);
    zf::DrawSurface(*zf_context.rendering_context, game->surf, {}, {1.0f, 1.0f});

    zf::SetSurface(*zf_context.rendering_context, game->surf);
    zf::DrawClear();
    zf::DrawRect(*zf_context.rendering_context, {0, 0, 32, 32}, {0.0f, 1.0f, 0.0f, 1.0f});
    zf::UnsetSurface(*zf_context.rendering_context);

    zf::SetSurfaceShaderProg(*zf_context.rendering_context, zf_context.rendering_context->basis->surf_default_shader_prog_hdl);
    zf::DrawSurface(*zf_context.rendering_context, game->surf, {20.0f, 20.0f}, {1.0f, 1.0f});

    // Render mouse.
    const auto mouse_pos = static_cast<zf::s_v2<zf::t_f32>>(zf::GetMousePos());
    const zf::s_rect<zf::t_f32> mouse_rect = {mouse_pos.x - (g_mouse_size / 2.0f), mouse_pos.y - (g_mouse_size / 2.0f), g_mouse_size, g_mouse_size};
    zf::DrawRect(*zf_context.rendering_context, mouse_rect, zf::colors::g_red);
#endif

    return true;
}

void GameCleanup(void* const dev_mem) {
    const auto game = static_cast<s_game*>(dev_mem);
}
