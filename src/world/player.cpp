#include "world.h"

#include <zc/gfx.h>

constexpr float g_player_vel_lerp = 0.2f;
constexpr float g_player_move_spd = 2.0f;
constexpr zf::s_v2<int> g_player_size = {32, 20};
constexpr zf::s_v2<float> g_player_origin = zf::origins::g_center;

void PlayerTick(s_world &world, const zf::s_game_tick_context& zf_context) {
    /*const zf::s_v2_s32 move_axis = {
        zf::IsKeyDown(zf_context.input_context, zf::ek_key_code_d) - zf::IsKeyDown(zf_context.input_context, zf::ek_key_code_a),
        zf::IsKeyDown(zf_context.input_context, zf::ek_key_code_s) - zf::IsKeyDown(zf_context.input_context, zf::ek_key_code_w)
    };

    const zf::s_v2 vel_targ = static_cast<zf::s_v2>(move_axis) * g_player_move_spd;
    world.player.vel = zf::Lerp(world.player.vel, vel_targ, g_player_vel_lerp);
    world.player.pos += world.player.vel;*/
}
