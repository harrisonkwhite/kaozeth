#include <stdlib.h>
#include <zfw_audio.h>
#include "game.h"
#include "zfw_utils.h"

#define RESPAWN_TIME 120

typedef struct {
    s_textures textures;
    s_fonts fonts;
    s_sound_types snd_types;

    s_title_screen title_screen;

    bool in_world;
    s_world world;
} s_game;

const s_sprite g_sprites[] = {
    [ek_sprite_player] = {
        .tex = ek_texture_player,
        .src_rect = {1, 1, 14, 22}
    },

    [ek_sprite_slime] = {
        .tex = ek_texture_npcs,
        .src_rect = {1, 1, 14, 14}
    },

    [ek_sprite_dirt_tile] = {
        .tex = ek_texture_tiles,
        .src_rect = {0, 0, 8, 8}
    },

    [ek_sprite_stone_tile] = {
        .tex = ek_texture_tiles,
        .src_rect = {8, 0, 8, 8}
    },

    [ek_sprite_grass_tile] = {
        .tex = ek_texture_tiles,
        .src_rect = {16, 0, 8, 8}
    },

    [ek_sprite_tile_break_0] = {
        .tex = ek_texture_tiles,
        .src_rect = {0, 8, 8, 8}
    },

    [ek_sprite_tile_break_1] = {
        .tex = ek_texture_tiles,
        .src_rect = {8, 8, 8, 8}
    },

    [ek_sprite_tile_break_2] = {
        .tex = ek_texture_tiles,
        .src_rect = {16, 8, 8, 8}
    },

    [ek_sprite_tile_break_3] = {
        .tex = ek_texture_tiles,
        .src_rect = {24, 8, 8, 8}
    },

    [ek_sprite_dirt_block_item_icon] = {
        .tex = ek_texture_item_icons,
        .src_rect = {1, 1, 6, 6}
    },

    [ek_sprite_stone_block_item_icon] = {
        .tex = ek_texture_item_icons,
        .src_rect = {9, 1, 6, 6}
    },

    [ek_sprite_grass_block_item_icon] = {
        .tex = ek_texture_item_icons,
        .src_rect = {17, 1, 6, 6}
    },

    [ek_sprite_copper_pickaxe_item_icon] = {
        .tex = ek_texture_item_icons,
        .src_rect = {2, 9, 12, 14}
    },

    [ek_sprite_item_icon_template] = {
        .tex = ek_texture_item_icons,
        .src_rect = {0, 24, 16, 16}
    },

    [ek_sprite_projectile] = {
        .tex = ek_texture_projectiles,
        .src_rect = {0, 2, 16, 4}
    },

    [ek_sprite_mouse] = {
        .tex = ek_texture_misc,
        .src_rect = {2, 2, 4, 4}
    }
};

static_assert(STATIC_ARRAY_LEN(g_sprites) == eks_sprite_cnt, "Invalid array length!");

static const char* TextureIndexToFilePath(const int index) {
    switch ((e_texture)index) {
        case ek_texture_player: return "assets/textures/player.png";
        case ek_texture_npcs: return "assets/textures/npcs.png";
        case ek_texture_tiles: return "assets/textures/tiles.png";
        case ek_texture_item_icons: return "assets/textures/item_icons.png";
        case ek_texture_projectiles: return "assets/textures/projectiles.png";
        case ek_texture_misc: return "assets/textures/misc.png";

        default:
            assert(false && "Texture case not handled!");
            return NULL;
    }
}

static s_font_load_info FontIndexToLoadInfo(const int index) {
    switch ((e_font)index) {
        case ek_font_eb_garamond_20:
            return (s_font_load_info){
                .file_path = "assets/fonts/eb_garamond.ttf",
                .height = 20
            };

        case ek_font_eb_garamond_24:
            return (s_font_load_info){
                .file_path = "assets/fonts/eb_garamond.ttf",
                .height = 24
            };

        case ek_font_eb_garamond_28:
            return (s_font_load_info){
                .file_path = "assets/fonts/eb_garamond.ttf",
                .height = 28
            };

        case ek_font_eb_garamond_32:
            return (s_font_load_info){
                .file_path = "assets/fonts/eb_garamond.ttf",
                .height = 32
            };

        case ek_font_eb_garamond_48:
            return (s_font_load_info){
                .file_path = "assets/fonts/eb_garamond.ttf",
                .height = 48
            };

        case ek_font_eb_garamond_80:
            return (s_font_load_info){
                .file_path = "assets/fonts/eb_garamond.ttf",
                .height = 80
            };

        default:
            assert(false && "Font case not handled!");
            return (s_font_load_info){0};
    }
}

static const char* SoundTypeIndexToFilePath(const int index) {
    switch ((e_sound_type)index) {
        case ek_sound_type_button_click: return "assets/audio/button_click.wav";
        case ek_sound_type_item_drop_collect: return "assets/audio/item_drop_collect.wav";

        default:
            assert(false && "Audio case not handled!");
            return NULL;
    }
}

static bool InitGame(const s_game_init_func_data* const func_data) {
    s_game* const game = func_data->user_mem;

    if (!LoadTexturesFromFiles(&game->textures, func_data->perm_mem_arena, eks_texture_cnt, TextureIndexToFilePath)) {
        return false;
    }

    if (!LoadFontsFromFiles(&game->fonts, func_data->perm_mem_arena, eks_font_cnt, FontIndexToLoadInfo, func_data->temp_mem_arena)) {
        return false;
    }

    if (!LoadSoundTypesFromFiles(&game->snd_types, func_data->perm_mem_arena, eks_sound_type_cnt, SoundTypeIndexToFilePath)) {
        return false;
    }

    if (!InitTitleScreen(&game->title_screen)) {
        return false;
    }

    return true;
}

static e_game_tick_func_result GameTick(const s_game_tick_func_data* const func_data) {
    s_game* const game = func_data->user_mem;

    if (game->in_world) {
        if (!WorldTick(&game->world, func_data->input_state, func_data->input_state_last, func_data->window_state.size, func_data->audio_sys, &game->snd_types)) {
            return ek_game_tick_func_result_error;
        }
    } else {
        const s_title_screen_tick_result tick_res = TitleScreenTick(&game->title_screen, func_data->input_state, func_data->input_state_last, func_data->unicode_buf, func_data->window_state.size, &game->fonts, func_data->audio_sys, &game->snd_types, func_data->temp_mem_arena);

        switch (tick_res.type) {
            case ek_title_screen_tick_result_type_error:
                return ek_game_tick_func_result_error;

            case ek_title_screen_tick_result_type_load_world:
                ZERO_OUT(game->title_screen);

                game->in_world = true;

                if (!InitWorld(&game->world, &tick_res.world_filename)) {
                    return ek_game_tick_func_result_error;
                }

                break;

            case ek_title_screen_tick_result_type_exit:
                return ek_game_tick_func_result_exit;

            default: break;
        }
    }

    return ek_game_tick_func_result_default;
}

static void InitUIViewMatrix(t_matrix_4x4* const mat) {
    assert(mat && IS_ZERO(*mat));

    InitIdenMatrix4x4(mat);
    ScaleMatrix4x4(mat, UI_SCALE);
}

static bool RenderGame(const s_game_render_func_data* const func_data) {
    s_game* const game = func_data->user_mem;

    RenderClear((s_color){0.2, 0.3, 0.4, 1.0});

    if (game->in_world) {
        RenderWorld(&func_data->rendering_context, &game->world, &game->textures);

        ZERO_OUT(func_data->rendering_context.state->view_mat);
        InitUIViewMatrix(&func_data->rendering_context.state->view_mat);

        if (!RenderWorldUI(&func_data->rendering_context, &game->world, func_data->input_state->mouse_pos, &game->textures, &game->fonts, func_data->temp_mem_arena)) {
            return false;
        }
    } else {
        ZERO_OUT(func_data->rendering_context.state->view_mat);
        InitUIViewMatrix(&func_data->rendering_context.state->view_mat);

        if (!RenderTitleScreen(&func_data->rendering_context, &game->title_screen, &game->textures, &game->fonts, func_data->temp_mem_arena)) {
            return false;
        }
    }

    // Render the mouse.
    const s_vec_2d mouse_ui_pos = DisplayToUIPos(func_data->input_state->mouse_pos);
    RenderSprite(&func_data->rendering_context, ek_sprite_mouse, &game->textures, mouse_ui_pos, (s_vec_2d){0.5f, 0.5f}, (s_vec_2d){1.0f, 1.0f}, 0.0f, WHITE);

    Flush(&func_data->rendering_context);

    return true;
}

static void CleanGame(void* const user_mem) {
    s_game* const game = user_mem;

    UnloadFonts(&game->fonts);
    UnloadTextures(&game->textures);
}

int main() {
    const s_game_info game_info = {
        .user_mem_size = sizeof(s_game),
        .user_mem_alignment = ALIGN_OF(s_game),

        .window_init_size = {1280, 720},
        .window_title = "Terraria Clone",
        .window_flags = ek_window_flags_hide_cursor | ek_window_flags_resizable,

        .init_func = InitGame,
        .tick_func = GameTick,
        .render_func = RenderGame,
        .clean_func = CleanGame,
    };

    return RunGame(&game_info) ? EXIT_SUCCESS : EXIT_FAILURE;
}
