#include "world.h"

#include "assets.h"

constexpr zcl::t_v2 k_player_size = {20.0f, 20.0f};
constexpr zcl::t_v2 k_player_origin = zcl::k_origin_center;
constexpr zcl::t_f32 k_player_spd = 2.0f;
constexpr zcl::t_f32 k_player_vel_lerp_factor = 0.2f;
constexpr zcl::t_i32 k_player_flash_time_max = 10;

void PlayerInit(t_world *const world) {
    world->player = {};
}

void PlayerTick(t_world *const world, const zgl::t_input_state *const input_state) {
    const zcl::t_b8 key_right = zgl::KeyCheckDown(input_state, zgl::ek_key_code_d);
    const zcl::t_b8 key_left = zgl::KeyCheckDown(input_state, zgl::ek_key_code_a);
    const zcl::t_b8 key_up = zgl::KeyCheckDown(input_state, zgl::ek_key_code_w);
    const zcl::t_b8 key_down = zgl::KeyCheckDown(input_state, zgl::ek_key_code_s);

    const zcl::t_v2 move_axis = {
        static_cast<zcl::t_f32>(key_right - key_left),
        static_cast<zcl::t_f32>(key_down - key_up),
    };

    const zcl::t_v2 vel_dest = move_axis * k_player_spd;
    world->player.vel = zcl::Lerp(world->player.vel, vel_dest, k_player_vel_lerp_factor);

    world->player.pos += world->player.vel;

    world->player.rot = zcl::CalcDirRads(world->player.pos, ScreenToWorldPos(zgl::CursorGetPos(input_state), world->camera_pos));

    if (world->player.flash_time > 0) {
        world->player.flash_time--;
    }

    if (zgl::KeyCheckPressed(input_state, zgl::ek_key_code_space)) {
        world->player.flash_time = k_player_flash_time_max;
    }
}

void PlayerRender(const t_player *const player, const zgl::t_frame_context frame_context, const t_assets *const assets) {
    if (player->flash_time > 0) {
        zgl::FrameSetShaderProg(frame_context, zgl::FrameGetBuiltinShaderProgBlend(frame_context));
        zgl::FrameSetUniformV4(frame_context, zgl::FrameGetBuiltinUniformBlend(frame_context), {1.0f, 1.0f, 1.0f, static_cast<zcl::t_f32>(player->flash_time) / k_player_flash_time_max});
    }

    zgl::FrameSubmitTexture(frame_context, assets->textures[ek_texture_id_temp], player->pos, k_texture_temp_src_rects[ek_texture_temp_src_rect_id_player], k_player_origin, player->rot);

    if (player->flash_time > 0) {
        zgl::FrameSetShaderProg(frame_context, nullptr);
    }
}

zcl::t_rect_f PlayerGetCollider(const zcl::t_v2 player_pos) {
    return zcl::RectCreateF(player_pos - zcl::CalcCompwiseProd(k_player_size, k_player_origin), k_player_size);
}
