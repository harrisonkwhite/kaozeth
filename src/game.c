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

#if 0
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
#endif

    return true;
}

static e_game_tick_func_result GameTick(const s_game_tick_func_data* const func_data) {
    s_game* const game = func_data->user_mem;

#if 0
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
#endif

    return ek_game_tick_func_result_default;
}

static void InitUIViewMatrix(t_matrix_4x4* const mat) {
    assert(mat && IS_ZERO(*mat));

    InitIdenMatrix4x4(mat);
    ScaleMatrix4x4(mat, UI_SCALE);
}

#define LIGHTMAP_WIDTH 32
#define LIGHTMAP_HEIGHT 32

typedef int t_lightmap[LIGHTMAP_HEIGHT][LIGHTMAP_WIDTH];

#define LIGHT_LIMIT (LIGHTMAP_WIDTH * LIGHTMAP_HEIGHT)

typedef struct {
    s_vec_2d_i buf[LIGHT_LIMIT];
    int start;
    int len;
} s_light_queue;

bool IsLightPosInBounds(const s_vec_2d_i pos) {
    return pos.x >= 0 && pos.x < LIGHTMAP_WIDTH && pos.y >= 0 && pos.y < LIGHTMAP_HEIGHT;
}

bool IsLightQueueValid(const s_light_queue* const queue) {
    if (queue->start < 0 || queue->start >= LIGHT_LIMIT || queue->len < 0 || queue->len > LIGHT_LIMIT) {
        return false;
    }

    for (int i = 0; i < queue->len; i++) {
        const int bi = (queue->start + i) % LIGHT_LIMIT;
        const s_vec_2d_i light = queue->buf[bi];

        if (!IsLightPosInBounds(light)) {
            return false;
        }
    }

    return true;
}

bool EnqueueLight(s_light_queue* const queue, const s_vec_2d_i light_pos) {
    assert(IsLightQueueValid(queue));
    assert(IsLightPosInBounds(light_pos));

    if (queue->len < LIGHT_LIMIT) {
        queue->buf[(queue->start + queue->len) % LIGHT_LIMIT] = light_pos;
        queue->len++;
        return true;
    }

    return false;
}

s_vec_2d_i DequeueLight(s_light_queue* const queue) {
    assert(IsLightQueueValid(queue));
    assert(queue->len > 0);

    const s_vec_2d_i light = queue->buf[queue->start];

    queue->start++;
    queue->start %= LIGHT_LIMIT;

    queue->len--;

    return light;
}

#define LIGHTMAP_LIGHT_LEVEL_LIMIT 16

static void LoadLightmap(t_lightmap* const map, const s_vec_2d_i init_light_pos) {
    assert(IS_ZERO(*map));
    assert(IsLightPosInBounds(init_light_pos));

    // NOTE: Modelled after BFS.
    s_light_queue queue = {0};
    EnqueueLight(&queue, init_light_pos);
    (*map)[init_light_pos.y][init_light_pos.x] = LIGHTMAP_LIGHT_LEVEL_LIMIT;

    EnqueueLight(&queue, (s_vec_2d_i){0});
    (*map)[0][0] = LIGHTMAP_LIGHT_LEVEL_LIMIT;

    while (queue.len > 0) {
        const s_vec_2d_i light_pos = DequeueLight(&queue);

        assert((*map)[light_pos.y][light_pos.x] > 0); // Sanity check.

        const s_vec_2d_i neighbour_offsets[4] = {
            {0, 1},
            {0, -1},
            {1, 0},
            {-1, 0},
        };

        for (int i = 0; i < STATIC_ARRAY_LEN(neighbour_offsets); i++) {
            const s_vec_2d_i n = Vec2DISum(light_pos, neighbour_offsets[i]);

            if (!IsLightPosInBounds(n)) {
                continue;
            }

            const int new_light_val = (*map)[light_pos.y][light_pos.x] - 1;

            if ((*map)[n.y][n.x] >= new_light_val) {
                continue;
            }

            (*map)[n.y][n.x] = new_light_val;

            if ((*map)[n.y][n.x] > 0) {
                EnqueueLight(&queue, n);
            }
        }
    }
}

#define LIGHTMAP_TILE_SIZE 32.0f

static bool RenderGame(const s_game_render_func_data* const func_data) {
    s_game* const game = func_data->user_mem;

    RenderClear((s_color){0.2, 0.3, 0.4, 1.0});

    t_lightmap lightmap = {0};

    const s_vec_2d_i mouse_light_pos = {
        func_data->input_state->mouse_pos.x / LIGHTMAP_TILE_SIZE,
        func_data->input_state->mouse_pos.y / LIGHTMAP_TILE_SIZE
    };

    if (IsLightPosInBounds(mouse_light_pos)) {
        LoadLightmap(&lightmap, mouse_light_pos);
    }

    for (int ty = 0; ty < LIGHTMAP_HEIGHT; ty++) {
        for (int tx = 0; tx < LIGHTMAP_WIDTH; tx++) {
            const int light_lvl = lightmap[ty][tx];

            if (light_lvl == LIGHTMAP_LIGHT_LEVEL_LIMIT) {
                continue;
            }

            const s_rect rect = {
                tx * LIGHTMAP_TILE_SIZE,
                ty * LIGHTMAP_TILE_SIZE,
                LIGHTMAP_TILE_SIZE,
                LIGHTMAP_TILE_SIZE
            };

            const s_color blend = {
                .a = 1.0f - ((float)light_lvl / LIGHTMAP_LIGHT_LEVEL_LIMIT)
            };

            RenderRect(&func_data->rendering_context, rect, blend);
        }
    }

#if 0
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
#endif

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
