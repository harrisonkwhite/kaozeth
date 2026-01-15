#include "assets.h"

t_assets AssetsLoadAll(zgl::t_gfx *const gfx, zcl::t_arena *const arena, zcl::t_arena *const temp_arena) {
    t_assets assets = {};

    assets.gfx_resource_group = zgl::GFXResourceGroupCreate(gfx, arena);

    for (zcl::t_i32 i = 0; i < ekm_texture_id_cnt; i++) {
        assets.textures[i] = zgl::TextureCreateFromPacked(gfx, g_texture_file_paths[i], assets.gfx_resource_group, temp_arena);
    }

    for (zcl::t_i32 i = 0; i < ekm_font_id_cnt; i++) {
        assets.fonts[i] = zgl::FontCreateFromPacked(gfx, g_font_file_paths[i], assets.gfx_resource_group, temp_arena);
    }

    assets.loaded = true;

    return assets;
}

void AssetsUnloadAll(t_assets *const assets, zgl::t_gfx *const gfx) {
    ZCL_ASSERT(assets->loaded);

    zgl::GFXResourceGroupDestroy(gfx, assets->gfx_resource_group);
    *assets = {};
}

const zgl::t_gfx_resource *AssetsGetTexture(const t_assets *const assets, const t_texture_id id) {
    return assets->textures[id];
}

const zgl::t_font *AssetsGetFont(const t_assets *const assets, const t_font_id id) {
    return &assets->fonts[id];
}
