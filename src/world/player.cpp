#include "world.h"

constexpr zf::t_f32 g_player_vel_lerp = 0.2f;
constexpr zf::t_f32 g_player_move_spd = 2.0f;
constexpr zf::s_v2<zf::t_s32> g_player_size = {32, 20};
constexpr zf::s_v2<zf::t_f32> g_player_origin = zf::origins::g_center;

void PlayerTick(s_world &world, const zf::s_game_tick_context& zf_context) {
}
