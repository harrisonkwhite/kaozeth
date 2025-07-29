#include "game.h"
#include "assets.h"
#include "zfw_graphics.h"
#include "zfw_math.h"

#include <stdio.h>

#define BG_COLOR (zfw_u_vec_4d){0.2, 0.3, 0.4, 1.0}

float g_ui_scale = 1.0f;

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

bool InitGame(const zfw_s_game_init_func_data* const func_data) {
    s_game* const game = func_data->user_mem;

    game->textures = ZFW_LoadTexturesFromFiles(func_data->perm_mem_arena, eks_texture_cnt, TextureIndexToFilePath);

    if (IS_ZERO(game->textures)) {
        return false;
    }

    game->fonts = ZFW_LoadFontsFromFiles(func_data->perm_mem_arena, eks_font_cnt, FontIndexToLoadInfo, func_data->temp_mem_arena);

    if (IS_ZERO(game->fonts)) {
        return false;
    }

    game->shader_progs = ZFW_CreateShaderProgsFromFiles(func_data->perm_mem_arena, eks_shader_prog_cnt, ShaderProgIndexToFilePaths, func_data->temp_mem_arena);

    if (IS_ZERO(game->shader_progs)) {
        return false;
    }

    if (!ZFW_LoadSoundTypesFromFiles(&game->snd_types, func_data->perm_mem_arena, eks_sound_type_cnt, SoundTypeIndexToFilePath)) {
        return false;
    }

    LoadSettings(&game->settings);

    if (!InitTitleScreen(&game->title_screen, func_data->temp_mem_arena)) {
        return false;
    }

    return true;
}

static inline float CalcUIScale(const zfw_s_vec_2d_s32 window_size) {
    if (window_size.x > 1920 && window_size.y > 1080) {
        return 2.0f;
    }

    if (window_size.x > 1600 && window_size.y > 900) {
        return 1.5f;
    }

    return 1.0f;
}

zfw_e_game_tick_func_result GameTick(const zfw_s_game_tick_func_data* const func_data) {
    s_game* const game = func_data->user_mem;

    g_ui_scale = CalcUIScale(func_data->window_state.size);

    if (game->in_world) {
        if (!WorldTick(&game->world, &game->settings, func_data->input_state, func_data->input_state_last, func_data->window_state.size, func_data->audio_sys, &game->snd_types)) {
            return ek_game_tick_func_result_error;
        }
    } else {
        const s_title_screen_tick_result tick_res = TitleScreenTick(&game->title_screen, &game->settings, func_data->input_state, func_data->input_state_last, func_data->unicode_buf, func_data->window_state.size, &game->fonts, func_data->audio_sys, &game->snd_types, func_data->temp_mem_arena);

        switch (tick_res.type) {
            case ek_title_screen_tick_result_type_error:
                return ek_game_tick_func_result_error;

            case ek_title_screen_tick_result_type_load_world:
                ZERO_OUT(game->title_screen);

                if (!InitWorld(&game->world, &tick_res.world_filename, func_data->window_state.size, func_data->temp_mem_arena)) {
                    return ek_game_tick_func_result_error;
                }

                game->in_world = true;

                break;

            case ek_title_screen_tick_result_type_exit:
                return ek_game_tick_func_result_exit;

            default: break;
        }
    }

    return ek_game_tick_func_result_default;
}

static void InitUIViewMatrix(zfw_t_matrix_4x4* const mat) {
    assert(mat && IS_ZERO(*mat));

    ZFW_InitIdenMatrix4x4(mat);
    ZFW_ScaleMatrix4x4(mat, g_ui_scale);
}

bool RenderGame(const zfw_s_game_render_func_data* const func_data) {
    s_game* const game = func_data->user_mem;

    ZFW_RenderClear(BG_COLOR);

    if (game->in_world) {
        if (!RenderWorld(&func_data->rendering_context, &game->world, &game->textures, &game->shader_progs, func_data->temp_mem_arena)) {
            return false;
        }

        ZERO_OUT(func_data->rendering_context.state->view_mat);
        InitUIViewMatrix(&func_data->rendering_context.state->view_mat);

        if (!RenderWorldUI(&func_data->rendering_context, &game->world, func_data->mouse_pos, &game->textures, &game->fonts, func_data->temp_mem_arena)) {
            return false;
        }
    } else {
        ZERO_OUT(func_data->rendering_context.state->view_mat);
        InitUIViewMatrix(&func_data->rendering_context.state->view_mat);

        if (!RenderTitleScreen(&func_data->rendering_context, &game->title_screen, &game->settings, &game->textures, &game->fonts, func_data->temp_mem_arena)) {
            return false;
        }
    }

    // Render the mouse.
    const zfw_s_vec_2d mouse_ui_pos = DisplayToUIPos(func_data->mouse_pos);
    RenderSprite(&func_data->rendering_context, ek_sprite_mouse, &game->textures, mouse_ui_pos, (zfw_s_vec_2d){0.5f, 0.5f}, (zfw_s_vec_2d){1.0f, 1.0f}, 0.0f, ZFW_WHITE);

    ZFW_SubmitBatch(&func_data->rendering_context);

    return true;
}

void CleanGame(void* const user_mem) {
    s_game* const game = user_mem;

    if (game->in_world) {
        CleanWorld(&game->world);
    }

    WriteSettingsToFile(&game->settings);

    ZFW_UnloadFonts(&game->fonts);
    ZFW_UnloadTextures(&game->textures);
}
