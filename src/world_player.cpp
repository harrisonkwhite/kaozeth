#include "world.h"

#include "assets.h"

namespace world {
    constexpr zf::math::t_v2_i k_player_size = {20, 20};
    constexpr zf::math::t_v2 k_player_origin = zf::rendering::k_origin_center;
    constexpr zf::t_f32 k_player_spd = 2.0f;
    constexpr zf::t_f32 k_player_vel_lerp_factor = 0.2f;

    void player_init(t_world *const world) {
        world->player = {};
    }

    void player_tick(t_world *const world, const zf::game::t_tick_func_context &zf_context) {
        const zf::t_b8 key_right = zf::input::key_check_down(zf_context.input_state, zf::input::ek_key_code_d);
        const zf::t_b8 key_left = zf::input::key_check_down(zf_context.input_state, zf::input::ek_key_code_a);
        const zf::t_b8 key_up = zf::input::key_check_down(zf_context.input_state, zf::input::ek_key_code_w);
        const zf::t_b8 key_down = zf::input::key_check_down(zf_context.input_state, zf::input::ek_key_code_s);

        const zf::math::t_v2 move_axis = {
            static_cast<zf::t_f32>(key_right - key_left),
            static_cast<zf::t_f32>(key_down - key_up),
        };

        const zf::math::t_v2 vel_dest = move_axis * k_player_spd;
        world->player.vel = zf::math::lerp(world->player.vel, vel_dest, k_player_vel_lerp_factor);

        world->player.pos += world->player.vel;

        world->player.rot = zf::math::calc_dir_in_rads(world->player.pos, zf::input::cursor_get_pos(zf_context.input_state));
    }

    void player_render(const t_player *const player, zf::rendering::t_frame_context *const frame_context) {
        zf::rendering::frame_submit_texture(frame_context, assets::get_texture(assets::ek_texture_id_temp), player->pos, assets::k_texture_temp_src_rects[assets::ek_texture_temp_src_rect_id_player], k_player_origin, player->rot);
    }

    zf::math::t_rect_f player_get_collider(const zf::math::t_v2 player_pos) {
        return zf::math::rect_create_f32(player_pos - (zf::math::v2_calc_compwise_prod(zf::math::v2_i_to_f(k_player_size), k_player_origin)), zf::math::v2_i_to_f(k_player_size));
    }
}
