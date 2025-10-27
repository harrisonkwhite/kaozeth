#include "game.h"
#include "zf_rendering.h"

bool GameInit(const zf::s_game_init_context& zf_context) {
    const auto game = static_cast<s_game*>(zf_context.dev_mem);

    const zf::s_static_array<zf::c_string_view, 1> tex_file_paths = {{
        "assets/textures/player.png"
    }};

    /*for (int i = 0; i < eks_texture_cnt; i++) {
        switch (i) {
            case 0:
                game->textures[i].LoadFromPacked();
                break;
        }
    }*/

    /*if (!game->tex_group.LoadRaws(tex_file_paths.Nonstatic(), zf_context.perm_mem_arena, zf::c_renderer::GFXResourceLifetime(), zf_context.temp_mem_arena)) {
        return false;
    }*/

    return true;
}

zf::e_game_tick_result GameTick(const zf::s_game_tick_context& zf_context) {
    const auto game = static_cast<s_game*>(zf_context.dev_mem);

    if (!WorldTick(game->world, zf_context)) {
        return zf::ek_game_tick_result_error;
    }

    return zf::ek_game_tick_result_normal;
}

bool GamePrerender(const zf::s_game_render_context& zf_context) {
    const auto game = static_cast<const s_game*>(zf_context.dev_mem);
    return true;
}

bool GameRender(const zf::s_game_render_context& zf_context) {
    const auto game = static_cast<const s_game*>(zf_context.dev_mem);

    //zf::c_renderer::Draw(0, game->tex_group, {32.0f, 32.0f}, {32.0f, 32.0f});

    return true;
}

void GameCleanup(void* const dev_mem) {
    const auto game = static_cast<s_game*>(dev_mem);
}
