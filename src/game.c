#include "game.h"

#include <stdio.h>
#include "title_screen.h"
#include "world/world.h"
#include "zfw_graphics.h"

float g_ui_scale = 1.0f;

typedef struct {
    zfw_s_textures textures;
    zfw_s_fonts fonts;
    zfw_s_sound_types snd_types;

    t_settings settings;

    s_title_screen title_screen;

    bool in_world;
    s_world world;
} s_game;

const s_projectile_type g_projectile_types[] = {
    [ek_projectile_type_wooden_arrow] = {
        .spr = ek_sprite_projectile,
        .flags = ek_projectile_type_flags_rot_is_dir
    }
};

static_assert(ZFW_STATIC_ARRAY_LEN(g_projectile_types) == eks_projectile_type_cnt, "Invalid array length!");

const s_tile_type g_tile_types[] = {
    [ek_tile_type_dirt] = {
        .spr = ek_sprite_dirt_tile,
        .drop_item = ek_item_type_dirt_block,
        .life = 5,
        .particle_template = ek_particle_template_dirt
    },
    [ek_tile_type_stone] = {
        .spr = ek_sprite_stone_tile,
        .drop_item = ek_item_type_stone_block,
        .life = 8,
        .particle_template = ek_particle_template_stone
    },
    [ek_tile_type_grass] = {
        .spr = ek_sprite_grass_tile,
        .drop_item = ek_item_type_grass_block,
        .life = 3,
        .particle_template = ek_particle_template_grass
    }
};

#define TILE_PLACE_DEFAULT_USE_BREAK 2

static_assert(ZFW_STATIC_ARRAY_LEN(g_tile_types) == eks_tile_type_cnt, "Invalid array length!");

const s_item_type g_item_types[] = {
    [ek_item_type_dirt_block] = {
        .name = "Dirt Block",
        .icon_spr = ek_sprite_dirt_block_item_icon,
        .use_type = ek_item_use_type_tile_place,
        .use_break = TILE_PLACE_DEFAULT_USE_BREAK,
        .consume_on_use = true,
        .tile_place_type = ek_tile_type_dirt
    },

    [ek_item_type_stone_block] = {
        .name = "Stone Block",
        .icon_spr = ek_sprite_stone_block_item_icon,
        .use_type = ek_item_use_type_tile_place,
        .use_break = TILE_PLACE_DEFAULT_USE_BREAK,
        .consume_on_use = true,
        .tile_place_type = ek_tile_type_stone
    },

    [ek_item_type_grass_block] = {
        .name = "Grass Block",
        .icon_spr = ek_sprite_grass_block_item_icon,
        .use_type = ek_item_use_type_tile_place,
        .use_break = TILE_PLACE_DEFAULT_USE_BREAK,
        .consume_on_use = true,
        .tile_place_type = ek_tile_type_grass
    },

    [ek_item_type_copper_pickaxe] = {
        .name = "Copper Pickaxe",
        .icon_spr = ek_sprite_copper_pickaxe_item_icon,
        .use_type = ek_item_use_type_tile_hurt,
        .use_break = 10,
        .tile_hurt_dist = 4
    },

    [ek_item_type_wooden_sword] = {
        .name = "Wooden Sword",
        .icon_spr = ek_sprite_item_icon_template,
        .use_type = ek_item_use_type_tile_place,
        .use_break = 10
    },

    [ek_item_type_wooden_bow] = {
        .name = "Wooden Bow",
        .icon_spr = ek_sprite_item_icon_template,
        .use_type = ek_item_use_type_shoot,
        .use_break = 10,
        .shoot_proj_type = ek_projectile_type_wooden_arrow,
        .shoot_proj_spd = 7.0f,
        .shoot_proj_dmg = 3
    }
};

static_assert(ZFW_STATIC_ARRAY_LEN(g_item_types) == eks_item_type_cnt, "Invalid array length!");

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

static_assert(ZFW_STATIC_ARRAY_LEN(g_settings) == eks_setting_cnt, "Invalid array length!");

static const t_settings g_settings_default = {
    [ek_setting_smooth_camera] = 1,
    [ek_setting_volume] = 100
};

static const char* SoundTypeIndexToFilePath(const int index) {
    switch ((e_sound_type)index) {
        case ek_sound_type_button_click: return "assets/audio/button_click.wav";
        case ek_sound_type_item_drop_collect: return "assets/audio/item_drop_collect.wav";

        default:
            assert(false && "Audio case not handled!");
            return NULL;
    }
}

static bool LoadSettingsFromFile(t_settings* const settings) {
    assert(ZFW_IS_ZERO(*settings));

    FILE* const fs = fopen(SETTINGS_FILENAME, "rb");

    if (!fs) {
        return false;
    }

    const int read = fread(settings, 1, sizeof(*settings), fs);

    fclose(fs);

    if (read < sizeof(*settings)) {
        ZFW_ZERO_OUT(*settings);
        return false;
    }

    return true;
}

static void LoadSettings(t_settings* const settings) {
    assert(ZFW_IS_ZERO(*settings));

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

static bool InitGame(const zfw_s_game_init_func_data* const func_data) {
    s_game* const game = func_data->user_mem;

    if (!ZFW_LoadTexturesFromFiles(&game->textures, func_data->perm_mem_arena, eks_texture_cnt, TextureIndexToFilePath)) {
        return false;
    }

    if (!ZFW_LoadFontsFromFiles(&game->fonts, func_data->perm_mem_arena, eks_font_cnt, FontIndexToLoadInfo, func_data->temp_mem_arena)) {
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

static inline float CalcUIScale(const zfw_s_vec_2d_i window_size) {
    if (window_size.x > 1920 && window_size.y > 1080) {
        return 2.0f;
    }

    if (window_size.x > 1600 && window_size.y > 900) {
        return 1.5f;
    }

    return 1.0f;
}

static zfw_e_game_tick_func_result GameTick(const zfw_s_game_tick_func_data* const func_data) {
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
                ZFW_ZERO_OUT(game->title_screen);

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
    assert(mat && ZFW_IS_ZERO(*mat));

    ZFW_InitIdenMatrix4x4(mat);
    ZFW_ScaleMatrix4x4(mat, g_ui_scale);
}

static bool RenderGame(const zfw_s_game_render_func_data* const func_data) {
    s_game* const game = func_data->user_mem;

    ZFW_RenderClear((zfw_s_vec_4d){0.2, 0.3, 0.4, 1.0});

    if (game->in_world) {
        if (!RenderWorld(&func_data->rendering_context, &game->world, &game->textures, func_data->temp_mem_arena)) {
            return false;
        }

        ZFW_ZERO_OUT(func_data->rendering_context.state->view_mat);
        InitUIViewMatrix(&func_data->rendering_context.state->view_mat);

        if (!RenderWorldUI(&func_data->rendering_context, &game->world, func_data->mouse_pos, &game->textures, &game->fonts, func_data->temp_mem_arena)) {
            return false;
        }
    } else {
        ZFW_ZERO_OUT(func_data->rendering_context.state->view_mat);
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

static void CleanGame(void* const user_mem) {
    s_game* const game = user_mem;

    if (game->in_world) {
        CleanWorld(&game->world);
    }

    WriteSettingsToFile(&game->settings);

    ZFW_UnloadFonts(&game->fonts);
    ZFW_UnloadTextures(&game->textures);
}

int main() {
    const zfw_s_game_info game_info = {
        .user_mem_size = sizeof(s_game),
        .user_mem_alignment = ZFW_ALIGN_OF(s_game),

        .window_init_size = {1280, 720},
        .window_title = GAME_TITLE,
        .window_flags = zfw_ek_window_flags_hide_cursor | zfw_ek_window_flags_resizable,

        .init_func = InitGame,
        .tick_func = GameTick,
        .render_func = RenderGame,
        .clean_func = CleanGame,
    };

    return ZFW_RunGame(&game_info) ? EXIT_SUCCESS : EXIT_FAILURE;
}
