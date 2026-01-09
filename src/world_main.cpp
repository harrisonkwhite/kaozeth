#include "world.h"

namespace world {
    constexpr zcl::gfx::t_color_rgba32f k_bg_color = zcl::gfx::color_create_rgba32f(0.43f, 0.73f, 1.0f);

    constexpr zcl::t_f32 k_camera_scale = 2.0f;

    static zcl::math::t_mat4x4 camera_create_view_matrix(const zcl::math::t_v2 cam_pos) {
        const zcl::math::t_mat4x4 scaling = zcl::math::matrix_create_scaled({k_camera_scale, k_camera_scale});

        const zcl::math::t_v2 cam_size = zcl::math::v2_i_to_f(zgl::platform::window_get_framebuffer_size_cache()) / k_camera_scale;
        const zcl::math::t_mat4x4 translation = zcl::math::matrix_create_translated(-cam_pos + (cam_size / 2.0f));

        return zcl::math::matrix_multiply(translation, scaling);
    }

    t_world create(zcl::mem::t_arena *const arena) {
        t_world result = {};

        result.rendering_resource_group = zgl::gfx::resource_group_create(arena);

        result.texture_target_all = zgl::gfx::texture_create_target(zgl::platform::window_get_framebuffer_size_cache() / k_camera_scale, &result.rendering_resource_group);

        enemy_spawn(&result, {100.0f, 100.0f});

        return result;
    }

    void destroy(t_world *const world) {
        zgl::gfx::resource_group_destroy(&world->rendering_resource_group);
    }

    void tick(t_world *const world, const zgl::game::t_tick_func_context &zf_context) {
        zgl::gfx::texture_resize_target_if_needed(world->texture_target_all, zgl::platform::window_get_framebuffer_size_cache() / k_camera_scale);

        player_tick(world, zf_context);
        enemies_tick(world);

        //
        // Camera Update
        //
        world->camera_pos = world->player.pos;
    }

    void render(const t_world *const world, zgl::gfx::t_frame_context *const frame_context, zcl::mem::t_arena *const temp_arena) {
        //
        // World
        //
        const zcl::math::t_mat4x4 camera_view_mat = camera_create_view_matrix(world->camera_pos);
        zgl::gfx::frame_pass_begin(frame_context, zgl::platform::window_get_framebuffer_size_cache(), camera_view_mat, true, k_bg_color);

        player_render(&world->player, frame_context);
        enemies_render(world, frame_context);

        zgl::gfx::frame_pass_end(frame_context);

#if 0
        // Render everything to an off-screen target, unscaled.
        {
            const zcl::math::t_mat4x4 view_mat = zcl::math::matrix_create_translated(-world->camera_pos + (zcl::math::v2_i_to_f(zgl::gfx::texture_get_size(world->texture_target_all)) / 2.0f));
            zgl::gfx::frame_pass_begin_offscreen(frame_context, world->texture_target_all, view_mat, true, k_bg_color);

            player_render(&world->player, frame_context);
            enemies_render(world, frame_context);

            zgl::gfx::frame_pass_end(frame_context);
        }

        // Render that target scaled up. This is to keep a crisp pixelated look.
        {
            const zcl::math::t_mat4x4 view_mat = zcl::math::matrix_create_scaled({k_camera_scale, k_camera_scale});
            zgl::gfx::frame_pass_begin(frame_context, zgl::platform::window_get_framebuffer_size_cache(), view_mat, true);

            zgl::gfx::frame_submit_texture(frame_context, world->texture_target_all, {});

            zgl::gfx::frame_pass_end(frame_context);
        }
#endif

        //
        // UI
        //
        zgl::gfx::frame_pass_begin(frame_context, zgl::platform::window_get_framebuffer_size_cache(), zcl::math::matrix_create_identity());

        zgl::gfx::frame_pass_end(frame_context);
    }
}
