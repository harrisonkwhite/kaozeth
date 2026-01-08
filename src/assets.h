#pragma once

namespace assets {
    // This actually is quite complicated.
    // So every texture has 1 or more frames.

    enum t_texture_id {
        ek_texture_id_temp,

        ekm_texture_id_cnt
    };

    inline const zf::t_static_array<zf::strs::t_str_rdonly, ekm_texture_id_cnt> g_texture_file_paths = {{
        ZF_STR_LITERAL("assets/textures/temp.dat"),
    }};

    enum t_texture_temp_src_rect_id {
        ek_texture_temp_src_rect_id_player,
        ek_texture_temp_src_rect_id_enemy,

        ekm_texture_temp_src_rect_id_cnt
    };

    constexpr zf::t_static_array<zf::math::t_rect_i, ekm_texture_temp_src_rect_id_cnt> k_texture_temp_src_rects = {{
        {4, 4, 24, 24},
        {32, 0, 16, 16},
    }};

    enum t_font_id {
        ek_font_id_eb_garamond_32,
        ek_font_id_eb_garamond_48,
        ek_font_id_eb_garamond_64,

        ekm_font_id_cnt
    };

    inline const zf::t_static_array<zf::strs::t_str_rdonly, ekm_font_id_cnt> g_font_file_paths = {{
        ZF_STR_LITERAL("assets/fonts/eb_garamond_32.dat"),
        ZF_STR_LITERAL("assets/fonts/eb_garamond_48.dat"),
        ZF_STR_LITERAL("assets/fonts/eb_garamond_64.dat"),
    }};

    void load_all(zf::mem::t_arena *const arena, zf::mem::t_arena *const temp_arena);
    void unload_all();

    const zf::rendering::t_resource *get_texture(const t_texture_id id);
    const zf::rendering::t_font *get_font(const t_font_id id);
}
