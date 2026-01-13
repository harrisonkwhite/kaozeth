#include "world.h"

#include "assets.h"

namespace world {
    constexpr zcl::t_v2_i k_player_size = {20, 20};
    constexpr zcl::t_v2 k_player_origin = zgl::gfx::k_origin_center;
    constexpr zcl::t_f32 k_player_spd = 2.0f;
    constexpr zcl::t_f32 k_player_vel_lerp_factor = 0.2f;

    void player_init(t_world *const world) {
        world->player = {};
    }

    void player_tick(t_world *const world, const zgl::game::t_tick_func_context &zf_context) {
        const zcl::t_b8 key_right = zgl::input::key_check_down(zf_context.input_state, zgl::input::ek_key_code_d);
        const zcl::t_b8 key_left = zgl::input::key_check_down(zf_context.input_state, zgl::input::ek_key_code_a);
        const zcl::t_b8 key_up = zgl::input::key_check_down(zf_context.input_state, zgl::input::ek_key_code_w);
        const zcl::t_b8 key_down = zgl::input::key_check_down(zf_context.input_state, zgl::input::ek_key_code_s);

        const zcl::t_v2 move_axis = {
            static_cast<zcl::t_f32>(key_right - key_left),
            static_cast<zcl::t_f32>(key_down - key_up),
        };

        const zcl::t_v2 vel_dest = move_axis * k_player_spd;
        world->player.vel = zcl::lerp(world->player.vel, vel_dest, k_player_vel_lerp_factor);

        world->player.pos += world->player.vel;

        world->player.rot = zcl::calc_dir_in_rads(world->player.pos, screen_to_world_pos(zgl::input::cursor_get_pos(zf_context.input_state), world->camera_pos));

        if (world->player.flash_time > 0) {
            world->player.flash_time--;
        }

        if (zgl::input::key_check_pressed(zf_context.input_state, zgl::input::ek_key_code_space)) {
            world->player.flash_time = k_player_flash_time_max;
        }
    }

    void player_render(const t_player *const player, zgl::gfx::t_frame_context *const frame_context) {
        if (player->flash_time > 0) {
            zgl::gfx::frame_set_shader_prog(frame_context, zgl::gfx::frame_get_builtin_shader_prog_blend(frame_context));
            zgl::gfx::frame_set_uniform_v4(frame_context, zgl::gfx::frame_get_builtin_uniform_blend(frame_context), {1.0f, 1.0f, 1.0f, static_cast<zcl::t_f32>(player->flash_time) / k_player_flash_time_max});
        }

        zgl::gfx::frame_submit_texture(frame_context, assets::get_texture(assets::ek_texture_id_temp), player->pos, assets::k_texture_temp_src_rects[assets::ek_texture_temp_src_rect_id_player], k_player_origin, player->rot);

        if (player->flash_time > 0) {
            zgl::gfx::frame_set_shader_prog(frame_context, nullptr);
        }
    }

    zcl::t_rect_f player_get_collider(const zcl::t_v2 player_pos) {
        return zcl::rect_create_f32(player_pos - (zcl::v2_calc_compwise_prod(zcl::v2_i_to_f(k_player_size), k_player_origin)), zcl::v2_i_to_f(k_player_size));
    }
}
