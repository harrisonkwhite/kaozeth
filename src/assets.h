#ifndef ASSETS_H
#define ASSETS_H

#include <stdbool.h>
#include <assert.h>
#include <zfw.h>

typedef enum {
    ek_texture_player,
    ek_texture_npcs,
    ek_texture_tiles,
    ek_texture_item_icons,
    ek_texture_projectiles,
    ek_texture_particles,
    ek_texture_misc,

    eks_texture_cnt
} e_texture;

static inline zfw_s_texture_info GenTextureInfo(const int tex_index, s_mem_arena* const mem_arena) {
    switch ((e_texture)tex_index) {
        case ek_texture_player:
            return ZFW_GenTextureInfoFromFile("assets/textures/player.png", mem_arena);

        case ek_texture_npcs:
            return ZFW_GenTextureInfoFromFile("assets/textures/npcs.png", mem_arena);

        case ek_texture_tiles:
            return ZFW_GenTextureInfoFromFile("assets/textures/tiles.png", mem_arena);

        case ek_texture_item_icons:
            return ZFW_GenTextureInfoFromFile("assets/textures/item_icons.png", mem_arena);

        case ek_texture_projectiles:
            return ZFW_GenTextureInfoFromFile("assets/textures/projectiles.png", mem_arena);

        case ek_texture_particles:
            return ZFW_GenTextureInfoFromFile("assets/textures/particles.png", mem_arena);

        case ek_texture_misc:
            return ZFW_GenTextureInfoFromFile("assets/textures/misc.png", mem_arena);

        default:
            assert(false && "Texture case not handled!");
            return (zfw_s_texture_info){0};
    }
}

typedef enum {
    ek_font_eb_garamond_20,
    ek_font_eb_garamond_24,
    ek_font_eb_garamond_28,
    ek_font_eb_garamond_32,
    ek_font_eb_garamond_48,
    ek_font_eb_garamond_80,

    eks_font_cnt
} e_font;

const static zfw_s_font_load_info g_font_load_infos[] = {
    [ek_font_eb_garamond_20] = {.file_path = "assets/fonts/eb_garamond.ttf", .height = 20},
    [ek_font_eb_garamond_24] = {.file_path = "assets/fonts/eb_garamond.ttf", .height = 24},
    [ek_font_eb_garamond_28] = {.file_path = "assets/fonts/eb_garamond.ttf", .height = 28},
    [ek_font_eb_garamond_32] = {.file_path = "assets/fonts/eb_garamond.ttf", .height = 32},
    [ek_font_eb_garamond_48] = {.file_path = "assets/fonts/eb_garamond.ttf", .height = 48},
    [ek_font_eb_garamond_80] = {.file_path = "assets/fonts/eb_garamond.ttf", .height = 80}
};

STATIC_ARRAY_LEN_CHECK(g_font_load_infos, eks_font_cnt);

typedef enum {
    ek_shader_prog_blend,
    ek_shader_prog_outline,

    eks_shader_prog_cnt
} e_shader_prog;

static inline zfw_s_shader_prog_info GenShaderProgInfo(const int prog_index, s_mem_arena* const mem_arena) {
    switch ((e_shader_prog)prog_index) {
        case ek_shader_prog_blend:
            {
                const char* const vs_src = (const char*)PushEntireFileContents("assets/shaders/blend.vert", mem_arena, true);

                if (!vs_src) {
                    return (zfw_s_shader_prog_info){0};
                }

                const char* const fs_src = (const char*)PushEntireFileContents("assets/shaders/blend.frag", mem_arena, true);

                if (!fs_src) {
                    return (zfw_s_shader_prog_info){0};
                }

                return (zfw_s_shader_prog_info){
                    .vs_src = vs_src,
                    .fs_src = fs_src
                };
            }

        case ek_shader_prog_outline:
            {
                const char* const vs_src = (const char*)PushEntireFileContents("assets/shaders/outline.vert", mem_arena, true);

                if (!vs_src) {
                    return (zfw_s_shader_prog_info){0};
                }

                const char* const fs_src = (const char*)PushEntireFileContents("assets/shaders/outline.frag", mem_arena, true);

                if (!fs_src) {
                    return (zfw_s_shader_prog_info){0};
                }

                return (zfw_s_shader_prog_info){
                    .vs_src = vs_src,
                    .fs_src = fs_src
                };
            }

        default:
            assert(false && "Shader program case not handled!");
            return (zfw_s_shader_prog_info){0};
    }
}

typedef enum {
    ek_sound_type_button_click,
    ek_sound_type_item_drop_collect,

    eks_sound_type_cnt
} e_sound_type;

static const char* const g_snd_type_file_paths[] = {
    [ek_sound_type_button_click] = "assets/audio/button_click.wav",
    [ek_sound_type_item_drop_collect] = "assets/audio/item_drop_collect.wav"
};

STATIC_ARRAY_LEN_CHECK(g_snd_type_file_paths, eks_sound_type_cnt);

#endif
