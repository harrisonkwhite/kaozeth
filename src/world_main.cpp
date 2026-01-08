#include "world.h"

namespace world {
    void init(t_world *const world, zf::mem::t_arena *const arena) {
        world->rendering_resource_group = zf::rendering::resource_group_create(arena);

        world->texture_target_all = zf::rendering::texture_create_target(zf::platform::window_get_framebuffer_size_cache(), &world->rendering_resource_group);

        enemy_spawn(world, {100.0f, 100.0f});
    }

    void deinit(t_world *const world) {
        zf::rendering::resource_group_destroy(&world->rendering_resource_group);
    }

    void tick(t_world *const world, const zf::game::t_tick_func_context &zf_context) {
        zf::rendering::texture_resize_target_if_needed(world->texture_target_all, zf::platform::window_get_framebuffer_size_cache());

        player_tick(world, zf_context);
        enemies_tick(world);

        //
        // Camera Update
        //
        world->camera_pos = world->player.pos;
    }

    void render(const t_world *const world, zf::rendering::t_frame_context *const frame_context) {
        zf::math::t_mat4x4 cam_view_mat = zf::math::matrix_create_scaled({2.0f, 2.0f});

        zf::rendering::frame_pass_configure(frame_context, 0, zf::platform::window_get_framebuffer_size_cache(), cam_view_mat, true);

        zf::rendering::frame_pass_configure_texture_target(frame_context, 1, world->texture_target_all, zf::math::matrix_create_identity(), true, zf::gfx::color_create_rgba32f(0.43f, 0.73f, 1.0f));

        zf::rendering::frame_pass_set(frame_context, 1);
        player_render(&world->player, frame_context);
        enemies_render(world, frame_context);

        zf::rendering::frame_pass_set(frame_context, 0);
        zf::rendering::frame_submit_texture(frame_context, world->texture_target_all, {});
    }
}
