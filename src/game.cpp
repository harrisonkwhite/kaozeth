#include "game.h"

constexpr zf::t_f32 g_player_spd = 3.0f;
constexpr zf::t_f32 g_player_vel_lerp = 0.2f;

static s_game g_game;

void GameInit(const zf::s_game_init_context &zf_context) {
    zf::SetWindowTitle(g_game_title, zf_context.temp_mem_arena);
    zf::SetCursorVisibility(false);

#if 0
    if (!zf::CreateTextureResourceFromPacked(zf::s_cstr_literal("assets/textures/player.zfd"), zf_context.temp_mem_arena, g_game.player_texture)) {
        ZF_FATAL();
    }

    if (!zf::CreateTextureResourceFromPacked(zf::s_cstr_literal("assets/textures/enemy.zfd"), zf_context.temp_mem_arena, g_game.enemy_texture)) {
        ZF_FATAL();
    }

    if (!zf::CreateFontResourceFromPacked(zf::s_cstr_literal("assets/fonts/eb_garamond_96.zfd"), zf_context.temp_mem_arena, g_game.font)) {
        ZF_FATAL();
    }
#endif
}

void GameTick(const zf::s_game_tick_context &zf_context) {
    if (zf::IsKeyPressed(zf::ek_key_code_f)) {
        zf::ToggleFullscreen();
    }

    const zf::s_v2 move_dir = {
        static_cast<zf::t_f32>(zf::IsKeyDown(zf::ek_key_code_d) - zf::IsKeyDown(zf::ek_key_code_a)),
        static_cast<zf::t_f32>(zf::IsKeyDown(zf::ek_key_code_s) - zf::IsKeyDown(zf::ek_key_code_w)),
    };

    const auto vel_targ = move_dir * g_player_spd;
    g_game.player.vel = zf::Lerp(g_game.player.vel, vel_targ, g_player_vel_lerp);
    g_game.player.pos += g_game.player.vel;

    g_game.player.rot = zf::CalcDirInRads(g_game.player.pos, zf::CursorPos());
}

void GameRender(const zf::s_game_render_context &zf_context) {
#if 0
    zf::DrawTexture(zf_context.rendering_context, *g_game.player_texture, g_game.player.pos);
    zf::DrawTexture(zf_context.rendering_context, *g_game.enemy_texture, g_game.player.pos + zf::s_v2(100.0f, 100.0f));
#endif
}

void GameCleanup() {}
