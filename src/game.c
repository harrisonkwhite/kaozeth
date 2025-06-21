#include <stdlib.h>
#include "game.h"

typedef struct {
    s_textures textures;
    s_fonts fonts;

    s_world world;
} s_game;

const s_rect_i g_sprite_src_rects[eks_sprite_cnt] = {
    {1, 1, 14, 22}, // Player
    {17, 9, 14, 14}, // Slime
    {16, 0, 8, 8}, // Tile
    {24, 0, 8, 8} // Cursor
};

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
        case ek_font_eb_garamond_36:
            return (s_font_load_info){
                .file_path = "assets/fonts/eb_garamond.ttf",
                .height = 36
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

    InitWorld(&game->world);

    return true;
}

static bool GameTick(const s_game_tick_func_data* const func_data) {
    s_game* const game = func_data->user_mem;

    WorldTick(&game->world, func_data->input_state, func_data->input_state_last, func_data->window_state.size);

    return true;
}

static bool RenderGame(const s_game_render_func_data* const func_data) {
    s_game* const game = func_data->user_mem;

    RenderWorld(&func_data->rendering_context, &game->world, &game->textures, &game->fonts, func_data->temp_mem_arena);

    // Render the cursor.
    RenderSprite(&func_data->rendering_context, ek_sprite_cursor, &game->textures, func_data->input_state->mouse_pos, (s_vec_2d){0.5f, 0.5f}, (s_vec_2d){1.0f, 1.0f}, 0.0f, WHITE);

    Flush(&func_data->rendering_context);

    return true;
}

static void CleanGame(void* const user_mem) {
}

int main() {
    const s_game_info game_info = {
        .user_mem_size = sizeof(s_game),
        .user_mem_alignment = alignof(s_game),

        .window_init_size = {1280, 720},
        .window_title = "Terraria Clone",
        .window_flags = ek_window_flag_hide_cursor | ek_window_flag_resizable,

        .init_func = InitGame,
        .tick_func = GameTick,
        .render_func = RenderGame,
        .clean_func = CleanGame,
    };

    return RunGame(&game_info) ? EXIT_SUCCESS : EXIT_FAILURE;
}
