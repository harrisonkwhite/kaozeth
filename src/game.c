#include <stdlib.h>
#include <zfw_game.h>

typedef enum {
    ek_sprite_player,

    eks_sprite_cnt
} e_sprite;

s_rect_i g_sprite_src_rects[eks_sprite_cnt] = {
    {0, 0, 24, 24}
};

typedef struct {
    s_textures textures;
} s_game;

static const char* TextureIndexToFilePath(const int index) {
    return "../assets/sprites.png";
}

static bool InitGame(const s_game_init_func_data* const func_data) {
    s_game* const game = func_data->user_mem;

    if (!LoadTexturesFromFiles(&game->textures, func_data->perm_mem_arena, 1, TextureIndexToFilePath)) {
            return false;
    }

    return true;
}

static bool GameTick(const s_game_tick_func_data* const func_data) {
    s_game* const game = func_data->user_mem;
    return true;
}

static inline void RenderSprite(const s_rendering_context* const context, const int sprite_index, const s_textures* const textures, const s_vec_2d pos, const s_vec_2d origin, const s_vec_2d scale, const float rot, const s_color blend) {
    RenderTexture(
        context,
        0,
        textures,
        g_sprite_src_rects[sprite_index],
        pos,
        origin,
        scale,
        rot,
        blend
    );
}

static bool RenderGame(const s_game_render_func_data* const func_data) {
    s_game* const game = func_data->user_mem;

    RenderClear((s_color){0.2, 0.3, 0.4, 1.0});

    RenderSprite(&func_data->rendering_context, ek_sprite_player, &game->textures, VEC_2D_ZERO, (s_vec_2d){0.5f, 0.5f}, (s_vec_2d){4.0f, 4.0f}, 0.0f, WHITE);
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
