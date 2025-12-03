#include "world.h"

constexpr zf::t_f32 g_player_vel_lerp = 0.2f;
constexpr zf::t_f32 g_player_move_spd = 2.0f;
constexpr zf::s_v2<zf::t_s32> g_player_size = {24, 40};
constexpr zf::s_v2<zf::t_f32> g_player_origin = zf::origins::g_center;

static zf::s_rect<zf::t_f32> MakePlayerRect(const zf::s_v2<zf::t_f32> player_pos) {
    //const auto size = static_cast<zf::s_v2<zf::t_f32>>(g_player_size);
    const auto size = static_cast<zf::s_v2<zf::t_f32>>(g_player_size);
    return {player_pos - (size / 2.0f), size};
}

void PlayerTick(s_world &world, const zf::s_game_tick_context& zf_context) {
#if 0
    const zf::t_b8 key_right = zf::IsKeyDown(zf::ek_key_code_d);
    const zf::t_b8 key_left = zf::IsKeyDown(zf::ek_key_code_a);
    const zf::t_b8 key_down = zf::IsKeyDown(zf::ek_key_code_s);
    const zf::t_b8 key_up = zf::IsKeyDown(zf::ek_key_code_w);

    const zf::s_v2<zf::t_f32> move_axis = {
        static_cast<zf::t_f32>(key_right) - static_cast<zf::t_f32>(key_left),
        static_cast<zf::t_f32>(key_down) - static_cast<zf::t_f32>(key_up)
    };

    const zf::s_v2<zf::t_f32> vel_targ = move_axis * g_player_move_spd;
    world.player.vel = zf::Lerp(world.player.vel, vel_targ, g_player_vel_lerp);

    world.player.pos += world.player.vel;
#endif
}

#if 0
void RenderPlayer(const s_player& player, const zf::s_rendering_context& rc) {
    const auto rect = MakePlayerRect(player.pos);
    zf::DrawRect(rc, rect, zf::colors::g_green);
}
#endif
