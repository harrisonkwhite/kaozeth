#pragma once

namespace assets {
    enum t_texture_id {
        ec_texture_id_temp,

        ecm_texture_id_cnt
    };

    inline const zf::t_static_array<zf::strs::t_str_rdonly, ecm_texture_id_cnt> g_texture_file_paths = {{
        ZF_STR_LITERAL("assets/fonts/eb_garamond_32.dat"),
    }};

    enum t_font_id {
        ec_font_id_eb_garamond_32,
        ec_font_id_eb_garamond_48,
        ec_font_id_eb_garamond_64,

        ecm_font_id_cnt
    };

    inline const zf::t_static_array<zf::strs::t_str_rdonly, ecm_font_id_cnt> g_font_file_paths = {{
        ZF_STR_LITERAL("assets/fonts/eb_garamond_32.dat"),
        ZF_STR_LITERAL("assets/fonts/eb_garamond_48.dat"),
        ZF_STR_LITERAL("assets/fonts/eb_garamond_64.dat"),
    }};

    void load_all(zf::mem::t_arena *const arena, zf::mem::t_arena *const temp_arena);
    void unload_all();

    const zf::rendering::t_resource *get_texture(const t_texture_id id);
    const zf::rendering::t_font *get_font(const t_font_id id);
}
