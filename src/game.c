#include "game.h"
#include "assets.h"

#include <stdio.h>

const s_setting g_settings[] = {
    [ek_setting_smooth_camera] = {
        .type = ek_setting_type_toggle,
        .name = "Smooth Camera"
    },
    [ek_setting_volume] = {
        .type = ek_setting_type_perc,
        .name = "Volume"
    }
};

STATIC_ARRAY_LEN_CHECK(g_settings, eks_setting_cnt);

static const t_settings g_settings_default = {
    [ek_setting_smooth_camera] = 1,
    [ek_setting_volume] = 100
};

static const zfw_s_vec_2d_int* PushSurfaceSizes(s_mem_arena* const mem_arena, const zfw_s_vec_2d_int window_size) {
    assert(mem_arena && IsMemArenaValid(mem_arena));
    assert(window_size.x > 0 && window_size.y > 0);

    zfw_s_vec_2d_int* const sizes = MEM_ARENA_PUSH_TYPE_CNT(mem_arena, zfw_s_vec_2d_int, eks_surface_cnt);

    for (int i = 0; i < eks_surface_cnt; i++) {
        switch ((e_surface)i) {
            case ek_surface_temp:
                sizes[i] = window_size;
                break;

            default:
                assert(false && "Surface case not handled!");
                break;
        }
    }

    return sizes;
}

static bool LoadSettingsFromFile(t_settings* const settings) {
    assert(IS_ZERO(*settings));

    FILE* const fs = fopen(SETTINGS_FILENAME, "rb");

    if (!fs) {
        return false;
    }

    const int read = fread(settings, 1, sizeof(*settings), fs);

    fclose(fs);

    if (read < sizeof(*settings)) {
        ZERO_OUT(*settings);
        return false;
    }

    return true;
}

static void LoadSettings(t_settings* const settings) {
    assert(IS_ZERO(*settings));

    if (!LoadSettingsFromFile(settings)) {
        // Failed to load settings file, so load defaults.
        memcpy(settings, g_settings_default, sizeof(t_settings));
    }
}

static bool WriteSettingsToFile(t_settings* const settings) {
    FILE* const fs = fopen(SETTINGS_FILENAME, "wb");

    if (!fs) {
        return false;
    }

    const int written = fwrite(settings, 1, sizeof(*settings), fs);

    fclose(fs);

    if (written != sizeof(*settings)) {
        return false;
    }

    return true;
}

bool InitGame(const zfw_s_game_init_context* const zfw_context) {
    s_game* const game = zfw_context->dev_mem;

    game->textures = ZFW_GenTextures(eks_texture_cnt, GenTextureInfo, zfw_context->gl_res_arena, zfw_context->perm_mem_arena, zfw_context->temp_mem_arena);

    if (IS_ZERO(game->textures)) {
        return false;
    }

    game->fonts = ZFW_GenFonts(eks_font_cnt, g_font_load_infos, zfw_context->gl_res_arena, zfw_context->perm_mem_arena, zfw_context->temp_mem_arena);

    if (IS_ZERO(game->fonts)) {
        return false;
    }

    game->shader_progs = ZFW_GenShaderProgs(eks_shader_prog_cnt, g_shader_prog_gen_infos, zfw_context->gl_res_arena, zfw_context->temp_mem_arena);

    if (IS_ZERO(game->shader_progs)) {
        return false;
    }

    {
        const zfw_s_vec_2d_int* const surf_sizes = PushSurfaceSizes(zfw_context->temp_mem_arena, zfw_context->window_state.size);

        if (!surf_sizes) {
            return false;
        }

        game->surfs = ZFW_GenSurfaces(eks_surface_cnt, surf_sizes, zfw_context->gl_res_arena, zfw_context->perm_mem_arena);

        if (IS_ZERO(game->surfs)) {
            return false;
        }
    }

    game->snd_types = ZFW_LoadSoundTypesFromFiles(zfw_context->perm_mem_arena, eks_sound_type_cnt, g_snd_type_file_paths);

    if (IS_ZERO(game->snd_types)) {
        return false;
    }

    LoadSettings(&game->settings);

    if (!InitTitleScreen(&game->title_screen, zfw_context->temp_mem_arena)) {
        return false;
    }

    return true;
}

zfw_e_game_tick_result GameTick(const zfw_s_game_tick_context* const zfw_context) {
    s_game* const game = zfw_context->dev_mem;

    {
        const zfw_s_vec_2d_int* const surf_sizes = PushSurfaceSizes(zfw_context->temp_mem_arena, zfw_context->window_state.size);

        if (!surf_sizes) {
            return zfw_ek_game_tick_result_error;
        }

        for (int i = 0; i < eks_surface_cnt; i++) {
            if (game->surfs.sizes[i].x != surf_sizes[i].x || game->surfs.sizes[i].y != surf_sizes[i].y) {
                ZFW_ResizeSurface(&game->surfs, i, surf_sizes[i]);
            }
        }
    }

    if (game->in_world) {
        if (!WorldTick(&game->world, &game->settings, zfw_context, &game->snd_types)) {
            return zfw_ek_game_tick_result_error;
        }
    } else {
        const s_title_screen_tick_result tick_res = TitleScreenTick(&game->title_screen, &game->settings, zfw_context, &game->fonts, &game->snd_types);

        switch (tick_res.type) {
            case ek_title_screen_tick_result_type_normal:
                break;

            case ek_title_screen_tick_result_type_error:
                return zfw_ek_game_tick_result_error;

            case ek_title_screen_tick_result_type_load_world:
                ZERO_OUT(game->title_screen);

                if (!InitWorld(&game->world, &tick_res.world_filename, zfw_context->window_state.size, zfw_context->temp_mem_arena)) {
                    return zfw_ek_game_tick_result_error;
                }

                game->in_world = true;

                break;

            case ek_title_screen_tick_result_type_exit:
                return zfw_ek_game_tick_result_exit;
        }
    }

    return zfw_ek_game_tick_result_normal;
}

static inline zfw_s_matrix_4x4 UIViewMatrix(const zfw_s_vec_2d_int window_size) {
    zfw_s_matrix_4x4 mat = ZFW_IdentityMatrix4x4();
    ZFW_ScaleMatrix4x4(&mat, UIScale(window_size));
    return mat;
}

bool RenderGame(const zfw_s_game_render_context* const zfw_context) {
    s_game* const game = zfw_context->dev_mem;

    const zfw_s_matrix_4x4 ui_view_matrix = UIViewMatrix(zfw_context->rendering_context.window_size);

    ZFW_Clear(&zfw_context->rendering_context, BG_COLOR);

    if (game->in_world) {
        if (!RenderWorld(&game->world, &zfw_context->rendering_context, &game->textures, &game->shader_progs, &game->surfs, zfw_context->temp_mem_arena)) {
            return false;
        }

        ZFW_SetViewMatrix(&zfw_context->rendering_context, &ui_view_matrix);

        if (!RenderWorldUI(&game->world, zfw_context, &game->textures, &game->fonts)) {
            return false;
        }
    } else {
        ZFW_SetViewMatrix(&zfw_context->rendering_context, &ui_view_matrix);

        if (!RenderTitleScreen(&game->title_screen, &zfw_context->rendering_context, &game->settings, &game->textures, &game->fonts, zfw_context->temp_mem_arena)) {
            return false;
        }
    }

    // Render the mouse.
    const zfw_s_vec_2d mouse_ui_pos = DisplayToUIPos(zfw_context->mouse_pos, zfw_context->rendering_context.window_size);
    RenderSprite(&zfw_context->rendering_context, ek_sprite_mouse, &game->textures, mouse_ui_pos, (zfw_s_vec_2d){0.5f, 0.5f}, (zfw_s_vec_2d){1.0f, 1.0f}, 0.0f, ZFW_WHITE);

    return true;
}

void CleanGame(void* const dev_mem) {
    s_game* const game = dev_mem;

    if (game->in_world) {
        CleanWorld(&game->world);
    }

    WriteSettingsToFile(&game->settings);
}
