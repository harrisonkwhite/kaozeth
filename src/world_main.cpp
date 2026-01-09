#include "world.h"

#include "assets.h"

namespace world {
    constexpr zf::gfx::t_color_rgba32f k_bg_color = zf::gfx::color_create_rgba32f(0.43f, 0.73f, 1.0f);

    constexpr zf::t_f32 k_camera_scale = 2.0f;

    static zf::math::t_v2_i calc_ideal_texture_target_all_size() {
        const zf::math::t_v2_i fb_size = zf::platform::window_get_framebuffer_size_cache();

        return {
            static_cast<zf::t_i32>(fb_size.x / k_camera_scale),
            static_cast<zf::t_i32>(fb_size.y / k_camera_scale),
        };
    }

    void init(t_world *const world, zf::mem::t_arena *const arena) {
        world->rendering_resource_group = zf::rendering::resource_group_create(arena);

        world->texture_target_all = zf::rendering::texture_create_target(calc_ideal_texture_target_all_size(), &world->rendering_resource_group);

        enemy_spawn(world, {100.0f, 100.0f});
    }

    void deinit(t_world *const world) {
        zf::rendering::resource_group_destroy(&world->rendering_resource_group);
    }

    void tick(t_world *const world, const zf::game::t_tick_func_context &zf_context) {
        zf::rendering::texture_resize_target_if_needed(world->texture_target_all, calc_ideal_texture_target_all_size());

        player_tick(world, zf_context);
        enemies_tick(world);

        //
        // Camera Update
        //
        world->camera_pos = world->player.pos;
    }

    void render(const t_world *const world, zf::rendering::t_frame_context *const frame_context) {
        // Render everything to an off-screen target, unscaled.
        zf::rendering::frame_pass_begin_offscreen(frame_context, world->texture_target_all, zf::math::matrix_create_identity(), true, k_bg_color);
        player_render(&world->player, frame_context);
        enemies_render(world, frame_context);
        zf::rendering::frame_pass_end(frame_context);

        // Render that target scaled up. This is to keep a crisp pixelated look.
        const zf::math::t_mat4x4 cam_view_mat = zf::math::matrix_create_scaled({k_camera_scale, k_camera_scale});
        zf::rendering::frame_pass_begin(frame_context, zf::platform::window_get_framebuffer_size_cache(), cam_view_mat, true);

        zf::rendering::frame_submit_texture(frame_context, world->texture_target_all, {});

        zf::rendering::frame_pass_end(frame_context);
    }

    void render_ui(const t_world *const world, zf::rendering::t_frame_context *const frame_context, zf::mem::t_arena *const temp_arena) {
    }
}
