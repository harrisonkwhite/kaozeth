#include "game.h"

constexpr zf::t_f32 g_player_spd = 3.0f;
constexpr zf::t_f32 g_player_vel_lerp = 0.2f;

static s_game g_game;

void GameInit(const zf::s_game_init_context &zf_context) {
    zf::SetWindowTitle(g_game_title, zf_context.temp_mem_arena);
    zf::SetCursorVisibility(false);

    if (!zf::PermGFXResourceArena().AddTextureFromPacked(zf::s_cstr_literal("assets/textures/player.zfd"), zf_context.temp_mem_arena, g_game.player_texture)) {
        ZF_FATAL();
    }

    if (!zf::PermGFXResourceArena().AddTextureFromPacked(zf::s_cstr_literal("assets/textures/enemy.zfd"), zf_context.temp_mem_arena, g_game.enemy_texture)) {
        ZF_FATAL();
    }

    if (!zf::CreateFontFromPacked(zf::s_cstr_literal("assets/fonts/arial.zfd"), zf_context.temp_mem_arena, g_game.font)) {
        ZF_FATAL();
    }

    if (!g_game.snd_type_arena.AddFromPacked(zf::s_cstr_literal("assets/sounds/explosion.zfd"), zf_context.temp_mem_arena, g_game.snd_types.explosion)) {
        ZF_FATAL();
    }
}

void GameTick(const zf::s_game_tick_context &zf_context) {
    if (zf::IsKeyPressed(zf_context.input_state, zf::ek_key_code_f)) {
        zf::ToggleFullscreen();
    }

    const zf::s_v2 move_dir = {
        static_cast<zf::t_f32>(zf::IsKeyDown(zf_context.input_state, zf::ek_key_code_d) - zf::IsKeyDown(zf_context.input_state, zf::ek_key_code_a)),
        static_cast<zf::t_f32>(zf::IsKeyDown(zf_context.input_state, zf::ek_key_code_s) - zf::IsKeyDown(zf_context.input_state, zf::ek_key_code_w)),
    };

    const auto vel_targ = move_dir * g_player_spd;
    g_game.player.vel = zf::Lerp(g_game.player.vel, vel_targ, g_player_vel_lerp);
    g_game.player.pos += g_game.player.vel;

    g_game.player.rot = zf::CalcDirInRads(g_game.player.pos, zf_context.input_state.cursor_pos);

    if (zf::IsKeyPressed(zf_context.input_state, zf::ek_key_code_space)) {
        zf::PlaySound(*g_game.snd_types.explosion);
    }

    for (zf::t_i32 i = 0; i < zf::g_gamepad_limit; i++) {
        if (zf::IsGamepadConnected(zf_context.input_state, i)) {
            zf::Log(zf::s_cstr_literal("Axis: %"), zf::GamepadAxisValueRaw(zf_context.input_state, i, zf::ek_gamepad_axis_code_left_x));
        }
    }
}

void GameRender(const zf::s_game_render_context &zf_context) {
    zf::DrawTexture(zf_context.rendering_context, *g_game.player_texture, g_game.player.pos);
    zf::DrawTexture(zf_context.rendering_context, *g_game.enemy_texture, g_game.player.pos + zf::s_v2(100.0f, 100.0f));

    zf::DrawStr(zf_context.rendering_context, zf::s_cstr_literal("Dude, man"), g_game.font, {}, zf_context.temp_mem_arena);
}

void GameCleanup() {
    g_game.snd_type_arena.Release();
}
