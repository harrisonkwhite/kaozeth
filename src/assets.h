#pragma once

enum t_texture_id {
    ek_texture_id_temp,

    ekm_texture_id_cnt
};

inline const zcl::t_static_array<zcl::t_str_rdonly, ekm_texture_id_cnt> g_texture_file_paths = {{
    ZCL_STR_LITERAL("assets/textures/temp.dat"),
}};

enum t_texture_temp_src_rect_id {
    ek_texture_temp_src_rect_id_player,
    ek_texture_temp_src_rect_id_enemy,

    ekm_texture_temp_src_rect_id_cnt
};

constexpr zcl::t_static_array<zcl::t_rect_i, ekm_texture_temp_src_rect_id_cnt> k_texture_temp_src_rects = {{
    {4, 4, 24, 24},
    {32, 0, 16, 16},
}};

enum t_font_id {
    ek_font_id_eb_garamond_32,
    ek_font_id_eb_garamond_48,
    ek_font_id_eb_garamond_64,
    ek_font_id_eb_garamond_128,
    ek_font_id_eb_garamond_256,

    ekm_font_id_cnt
};

inline const zcl::t_static_array<zcl::t_str_rdonly, ekm_font_id_cnt> g_font_file_paths = {{
    ZCL_STR_LITERAL("assets/fonts/eb_garamond_32.dat"),
    ZCL_STR_LITERAL("assets/fonts/eb_garamond_48.dat"),
    ZCL_STR_LITERAL("assets/fonts/eb_garamond_64.dat"),
    ZCL_STR_LITERAL("assets/fonts/eb_garamond_128.dat"),
    ZCL_STR_LITERAL("assets/fonts/eb_garamond_256.dat"),
}};

struct t_assets {
    zcl::t_b8 loaded;

    zgl::t_gfx_resource_group *gfx_resource_group;

    zcl::t_static_array<zgl::t_gfx_resource *, ekm_texture_id_cnt> textures;
    zcl::t_static_array<zgl::t_font, ekm_font_id_cnt> fonts;
};

t_assets AssetsLoadAll(zgl::t_gfx *const gfx, zcl::t_arena *const arena, zcl::t_arena *const temp_arena);
void AssetsUnloadAll(t_assets *const assets, zgl::t_gfx *const gfx);

const zgl::t_gfx_resource *AssetsGetTexture(const t_assets *const assets, const t_texture_id id);
const zgl::t_font *AssetsGetFont(const t_assets *const assets, const t_font_id id);
