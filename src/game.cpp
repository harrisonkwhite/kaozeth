#include "game.h"

zf::t_b8 GameInit(const zf::s_game_init_context& zf_context) {
    const auto game = static_cast<s_game*>(zf_context.dev_mem);

    if (!InitTitleScreen(game->state_data.ts)) {
        ZF_REPORT_ERROR();
        return false;
    }

    if (!zf::LoadTextureFromPacked("assets/textures/test.zfdat", *zf_context.gfx_res_arena, *zf_context.temp_mem_arena, game->tex)) {
        ZF_REPORT_ERROR();
        return false;
    }

    if (!zf::LoadFontFromPacked("assets/fonts/eb_garamond_96.zfdat", *zf_context.gfx_res_arena, *zf_context.temp_mem_arena, game->font)) {
        ZF_REPORT_ERROR();
        return false;
    }

    return true;
}

zf::t_b8 GameTick(const zf::s_game_tick_context& zf_context) {
    const auto game = static_cast<s_game*>(zf_context.dev_mem);
    return true;
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

    zf::DrawTexture(*zf_context.rendering_context, game->tex, {});

    if (!zf::DrawStr(*zf_context.rendering_context, "djkahdjksad", game->font, {200.0f, 200.0f}, {}, zf::colors::g_white, *zf_context.temp_mem_arena)) {
        return false;
    }

    zf::DrawRect(*zf_context.rendering_context, {0, 0, 32, 32}, zf::colors::g_red);

#if 0
    zf::renderer::SetSurface(game->surf);

    zf::renderer::Clear();
    zf::renderer::DrawRect({0, 0, 32, 32}, zf::colors::g_red);

    zf::renderer::UnsetSurface();

    zf::renderer::DrawSurface(game->surf, {});

    // Render mouse.
    const auto mouse_pos = static_cast<zf::s_v2<zf::t_f32>>(zf::MousePos());
    const zf::s_rect<zf::t_f32> mouse_rect = {mouse_pos.x - (g_mouse_size / 2.0f), mouse_pos.y - (g_mouse_size / 2.0f), g_mouse_size, g_mouse_size};
    zf::renderer::DrawRect(mouse_rect, zf::colors::g_red);
#endif

    return true;
}

void GameCleanup(void* const dev_mem) {
    const auto game = static_cast<s_game*>(dev_mem);
}
