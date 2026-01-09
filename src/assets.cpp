#include "assets.h"

namespace assets {
    struct {
        zf::t_b8 loaded;

        zgl::gfx::t_resource_group rendering_resource_group;

        zf::t_static_array<zgl::gfx::t_resource *, ekm_texture_id_cnt> textures;

        zf::t_static_array<zgl::gfx::t_font, ekm_font_id_cnt> fonts;
    } g_module_state;

    void load_all(zf::mem::t_arena *const arena, zf::mem::t_arena *const temp_arena) {
        ZF_ASSERT(!g_module_state.loaded);

        g_module_state = {};

        g_module_state.rendering_resource_group = zgl::gfx::resource_group_create(arena);

        for (zf::t_i32 i = 0; i < ekm_texture_id_cnt; i++) {
            g_module_state.textures[i] = zgl::gfx::texture_create_from_packed(g_texture_file_paths[i], temp_arena, &g_module_state.rendering_resource_group);
        }

        for (zf::t_i32 i = 0; i < ekm_font_id_cnt; i++) {
            g_module_state.fonts[i] = zgl::gfx::font_create_from_packed(g_font_file_paths[i], temp_arena, &g_module_state.rendering_resource_group);
        }

        g_module_state.loaded = true;
    }

    void unload_all() {
        ZF_ASSERT(g_module_state.loaded);

        zgl::gfx::resource_group_destroy(&g_module_state.rendering_resource_group);

        g_module_state = {};
    }

    const zgl::gfx::t_resource *get_texture(const t_texture_id id) {
        ZF_ASSERT(g_module_state.loaded);
        return g_module_state.textures[id];
    }

    const zgl::gfx::t_font *get_font(const t_font_id id) {
        ZF_ASSERT(g_module_state.loaded);
        return &g_module_state.fonts[id];
    }
}
