#include "world.h"

namespace world {
    void init(t_world *const world) {
    }

    void tick(t_world *const world, const zf::game::t_tick_func_context &zf_context) {
        player_tick(world, zf_context);
    }

    void render(t_world *const world, zf::rendering::t_frame_context *const frame_context) {
        zf::math::t_mat4x4 cam_view_mat = zf::math::k_mat4x4_identity;
        cam_view_mat.elems[0][0] *= 2.0f;
        cam_view_mat.elems[1][1] *= 2.0f;

        zf::rendering::frame_pass_configure(frame_context, 0, zf::platform::window_get_framebuffer_size_cache(), cam_view_mat, true, zf::gfx::color_create_rgba32f(0.43f, 0.73f, 1.0f));

        zf::rendering::frame_pass_configure(frame_context, 1, zf::platform::window_get_framebuffer_size_cache());

        zf::rendering::frame_pass_set(frame_context, 0);
        player_render(&world->player, frame_context);
    }
}
