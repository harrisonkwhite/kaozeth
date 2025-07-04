#include <stdlib.h>
#include "game.h"

#define RESPAWN_TIME 120

typedef struct {
    s_textures textures;
    s_fonts fonts;

    s_title_screen title_screen;

    bool in_world;
    s_world world;
} s_game;

const s_rect_i g_sprite_src_rects[] = {
    [ek_sprite_player] = {1, 1, 14, 22},
    [ek_sprite_slime] = {17, 9, 14, 14},
    [ek_sprite_tile_break_0] = {16, 24, 8, 8},
    [ek_sprite_tile_break_1] = {24, 24, 8, 8},
    [ek_sprite_tile_break_2] = {32, 24, 8, 8},
    [ek_sprite_tile_break_3] = {40, 24, 8, 8},
    [ek_sprite_dirt_tile] = {16, 0, 8, 8},
    [ek_sprite_stone_tile] = {32, 8, 8, 8},
    [ek_sprite_dirt_tile_item] = {33, 1, 6, 6},
    [ek_sprite_stone_tile_item] = {41, 1, 6, 6},
    [ek_sprite_pickaxe_item] = {49, 1, 14, 14},
    [ek_sprite_projectile] = {48, 18, 16, 4},
    [ek_sprite_cursor] = {24, 0, 8, 8}
};

static_assert(STATIC_ARRAY_LEN(g_sprite_src_rects) == eks_sprite_cnt, "Invalid array length!");

s_rect ColliderFromSprite(const e_sprite sprite, const s_vec_2d pos, const s_vec_2d origin) {
    const s_rect_i src_rect = g_sprite_src_rects[sprite];
    return (s_rect){
        pos.x - (src_rect.width * origin.x),
        pos.y - (src_rect.height * origin.y),
        src_rect.width,
        src_rect.height
    };
}

static const char* TextureIndexToFilePath(const int index) {
    return "assets/sprites.png";
}

static s_font_load_info FontIndexToLoadInfo(const int index) {
    switch (index) {
        case ek_font_eb_garamond_24:
            return (s_font_load_info){
                .file_path = "assets/fonts/eb_garamond.ttf",
                .height = 24
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
            return (s_font_load_info){0};
    }
}

static bool InitGame(const s_game_init_func_data* const func_data) {
    s_game* const game = func_data->user_mem;

    if (!LoadTexturesFromFiles(&game->textures, func_data->perm_mem_arena, 1, TextureIndexToFilePath)) {
        return false;
    }

    if (!LoadFontsFromFiles(&game->fonts, func_data->perm_mem_arena, eks_font_cnt, FontIndexToLoadInfo, func_data->temp_mem_arena)) {
        return false;
    }

    if (!InitTitleScreen(&game->title_screen, func_data->perm_mem_arena)) {
        return false;
    }

    return true;
}

static bool GameTick(const s_game_tick_func_data* const func_data) {
    s_game* const game = func_data->user_mem;

    if (game->in_world) {
        if (!WorldTick(&game->world, func_data->input_state, func_data->input_state_last, func_data->window_state.size)) {
            return false;
        }
    } else {
        const s_title_screen_tick_result tick_res = TitleScreenTick(&game->title_screen, func_data->input_state, func_data->input_state_last, func_data->window_state.size, &game->fonts, func_data->temp_mem_arena);

        switch (tick_res.type) {
            case ek_title_screen_tick_result_type_error:
                return false;

            case ek_title_screen_tick_result_type_load_world:
                ZERO_OUT(game->title_screen);

                game->in_world = true;

                if (!InitWorld(&game->world, tick_res.world_filename)) {
                    return false;
                }

                break;

            case ek_title_screen_tick_result_type_exit:
                break;

            default: break;
        }
    }

    return true;
}

static void InitUIViewMatrix(t_matrix_4x4* const mat) {
    assert(mat && IS_ZERO(*mat));

    InitIdenMatrix4x4(mat);
    ScaleMatrix4x4(mat, UI_SCALE);
}

static bool RenderGame(const s_game_render_func_data* const func_data) {
    s_game* const game = func_data->user_mem;

    RenderClear((s_color){0.2, 0.3, 0.4, 1.0});

    const s_vec_2d cursor_ui_pos = DisplayToUIPos(func_data->input_state->mouse_pos);

    if (game->in_world) {
        RenderWorld(&func_data->rendering_context, &game->world, &game->textures);

        ZERO_OUT(func_data->rendering_context.state->view_mat);
        InitUIViewMatrix(&func_data->rendering_context.state->view_mat);

        if (!RenderWorldUI(&func_data->rendering_context, &game->world, cursor_ui_pos, &game->textures, &game->fonts, func_data->temp_mem_arena)) {
            return false;
        }
    } else {
        ZERO_OUT(func_data->rendering_context.state->view_mat);
        InitUIViewMatrix(&func_data->rendering_context.state->view_mat);

        if (!RenderTitleScreen(&func_data->rendering_context, &game->title_screen, &game->textures, &game->fonts, func_data->temp_mem_arena)) {
            return false;
        }
    }

    // Render the cursor.
    RenderSprite(&func_data->rendering_context, ek_sprite_cursor, &game->textures, cursor_ui_pos, (s_vec_2d){0.5f, 0.5f}, (s_vec_2d){1.0f, 1.0f}, 0.0f, WHITE);

    Flush(&func_data->rendering_context);

    return true;
}

static void CleanGame(void* const user_mem) {
}

int main() {
    const s_game_info game_info = {
        .user_mem_size = sizeof(s_game),
        .user_mem_alignment = alignof(s_game),

        .window_init_size = {1920, 1080},
        .window_title = "Terraria Clone",
        .window_flags = ek_window_flag_hide_cursor | ek_window_flag_resizable,

        .init_func = InitGame,
        .tick_func = GameTick,
        .render_func = RenderGame,
        .clean_func = CleanGame,
    };

    return RunGame(&game_info) ? EXIT_SUCCESS : EXIT_FAILURE;
}
