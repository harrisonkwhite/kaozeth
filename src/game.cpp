#include "game.h"

constexpr zf::s_v2<zf::t_f32> g_player_size = {32.0f, 32.0f};
constexpr zf::t_f32 g_player_spd = 3.0f;
constexpr zf::t_f32 g_player_vel_lerp = 0.2f;

zf::t_b8 GameInit(const zf::s_game_init_context& zf_context) {
    const auto game = static_cast<s_game*>(zf_context.dev_mem);
    return true;
}

zf::t_b8 GameTick(const zf::s_game_tick_context& zf_context) {
    const auto game = static_cast<s_game*>(zf_context.dev_mem);

    const zf::s_v2<zf::t_f32> move_dir = {
        static_cast<zf::t_f32>(zf::IsKeyDown(zf::ek_key_code_d) - zf::IsKeyDown(zf::ek_key_code_a)),
        static_cast<zf::t_f32>(zf::IsKeyDown(zf::ek_key_code_s) - zf::IsKeyDown(zf::ek_key_code_w))
    };

    const auto vel_targ = move_dir * g_player_spd;
    game->player.vel = zf::Lerp(game->player.vel, vel_targ, g_player_vel_lerp);
    game->player.pos += game->player.vel;

    game->player.rot = zf::CalcDirInRads(game->player.pos, zf::MousePos());

    return true;
}

zf::t_b8 GameRender(const zf::s_game_render_context& zf_context) {
    const auto game = static_cast<const s_game*>(zf_context.dev_mem);

    const auto& rc = *zf_context.rendering_context;

    zf::DrawRectRot(rc, game->player.pos, g_player_size, zf::origins::g_center, game->player.rot, zf::colors::g_red);

    return true;
}

void GameCleanup(void* const dev_mem) {
    const auto game = static_cast<s_game*>(dev_mem);
}
