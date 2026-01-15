#include "world.h"

constexpr zcl::t_color_rgba32f k_bg_color = zcl::ColorCreateRGBA32F(0.43f, 0.73f, 1.0f);

static zcl::t_mat4x4 CameraCreateViewMatrix(const zcl::t_v2 cam_pos) {
    const zcl::t_mat4x4 scaling = zcl::MatrixCreateScaled({k_camera_scale, k_camera_scale});
    const zcl::t_mat4x4 translation = zcl::MatrixCreateTranslated(-cam_pos);
    return zcl::MatrixMultiply(translation, scaling);
}

t_world WorldCreate(zgl::t_gfx *const gfx, const zgl::t_platform *const platform, zcl::t_arena *const arena) {
    t_world result = {};

    result.gfx_resource_group = zgl::GFXResourceGroupCreate(gfx, arena);

    result.texture_target_all = zgl::TextureCreateTarget(gfx, zgl::WindowGetFramebufferSizeCache(platform) / k_camera_scale, result.gfx_resource_group);

    EnemySpawn(&result, {100.0f, 100.0f});

    return result;
}

void WorldDestroy(t_world *const world, zgl::t_gfx *const gfx) {
    zgl::GFXResourceGroupDestroy(gfx, world->gfx_resource_group);
}

void WorldTick(t_world *const world, const zgl::t_game_tick_func_context &zf_context) {
    const zcl::t_v2_i screen_size = zgl::WindowGetFramebufferSizeCache(zf_context.platform);

    zgl::TextureResizeTargetIfNeeded(zf_context.gfx, world->texture_target_all, screen_size / k_camera_scale);

    if (zgl::KeyCheckPressed(zf_context.input_state, zgl::ek_key_code_escape)) {
        world->paused = !world->paused;
    }

    if (world->paused) {
        return;
    }

    PlayerTick(world, zf_context.input_state);
    EnemiesTick(world);

    world->camera_pos = world->player.pos - (CameraGetSize(screen_size) / 2.0f);
}

void WorldRender(const t_world *const world, const zgl::t_frame_context frame_context, const t_assets *const assets, zcl::t_arena *const temp_arena) {

    //
    // World
    //
    const zcl::t_mat4x4 camera_view_mat = CameraCreateViewMatrix(world->camera_pos);
    zgl::FramePassBegin(frame_context, zgl::FrameGetSize(frame_context), camera_view_mat, true, k_bg_color);

    PlayerRender(&world->player, frame_context, assets);
    EnemiesRender(world, frame_context, assets);

    zgl::FramePassEnd(frame_context);

    //
    // UI
    //
    zgl::FramePassBegin(frame_context, zgl::FrameGetSize(frame_context), zcl::MatrixCreateIdentity());

    if (world->paused) {
        zgl::FrameSubmitRect(frame_context, zcl::RectCreateF({}, zcl::V2IToF(zgl::FrameGetSize(frame_context))), {0.0f, 0.0f, 0.0f, 0.8f});
    }

    zgl::FramePassEnd(frame_context);
}
