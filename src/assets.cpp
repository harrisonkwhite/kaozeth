#include "assets.h"

namespace assets {
    struct {
        zcl::t_b8 loaded;

        zgl::gfx::t_resource_group rendering_resource_group;

        zcl::t_static_array<zgl::gfx::t_resource *, ekm_texture_id_cnt> textures;

        zcl::t_static_array<zgl::gfx::t_font, ekm_font_id_cnt> fonts;
    } g_module_state;

    void load_all(zcl::t_arena *const arena, zcl::t_arena *const temp_arena) {
        ZCL_ASSERT(!g_module_state.loaded);

        g_module_state = {};

        g_module_state.rendering_resource_group = zgl::gfx::resource_group_create(arena);

        for (zcl::t_i32 i = 0; i < ekm_texture_id_cnt; i++) {
            g_module_state.textures[i] = zgl::gfx::texture_create_from_packed(g_texture_file_paths[i], temp_arena, &g_module_state.rendering_resource_group);
        }

        for (zcl::t_i32 i = 0; i < ekm_font_id_cnt; i++) {
            g_module_state.fonts[i] = zgl::gfx::font_create_from_packed(g_font_file_paths[i], temp_arena, &g_module_state.rendering_resource_group);
        }

        g_module_state.loaded = true;
    }

    void unload_all() {
        ZCL_ASSERT(g_module_state.loaded);

        zgl::gfx::resource_group_destroy(&g_module_state.rendering_resource_group);

        g_module_state = {};
    }

    const zgl::gfx::t_resource *get_texture(const t_texture_id id) {
        ZCL_ASSERT(g_module_state.loaded);
        return g_module_state.textures[id];
    }

    const zgl::gfx::t_font *get_font(const t_font_id id) {
        ZCL_ASSERT(g_module_state.loaded);
        return &g_module_state.fonts[id];
    }
}
