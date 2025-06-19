#include <stdlib.h>
#include <zfw_game.h>

#define GRAVITY 0.2f

#define PLAYER_MOVE_SPD 3.0f
#define PLAYER_MOVE_SPD_LERP 0.3f
#define PLAYER_JUMP_HEIGHT 5.0f

#define TILEMAP_WIDTH 32
#define TILEMAP_HEIGHT 32
#define TILE_SIZE 8
#define TILEMAP_SURFACE_LEVEL 24
static_assert(TILEMAP_SURFACE_LEVEL >= 0 && TILEMAP_SURFACE_LEVEL <= TILEMAP_HEIGHT, "Invalid tilemap surface level!");

#define VIEW_SCALE 3.0f

typedef enum {
    ek_sprite_player,
    ek_sprite_tile,

    eks_sprite_cnt
} e_sprite;

s_rect_i g_sprite_src_rects[eks_sprite_cnt] = {
    {0, 0, 24, 24}, // Player
    {24, 0, 8, 8} // Tile
};

typedef t_byte t_tilemap_activity[BITS_TO_BYTES(TILEMAP_HEIGHT)][BITS_TO_BYTES(TILEMAP_WIDTH)];

typedef struct {
    s_textures textures;

    s_vec_2d player_pos;
    s_vec_2d player_vel;

    t_tilemap_activity tilemap_activity;
} s_game;

static const char* TextureIndexToFilePath(const int index) {
    return "../assets/sprites.png";
}

static bool IsTileActive(const t_tilemap_activity* const tm_activity, const s_vec_2d_i pos) {
    assert(tm_activity);
    assert(pos.x >= 0 && pos.x < TILEMAP_WIDTH && pos.y >= 0 && pos.y < TILEMAP_HEIGHT);
    return IsBitActive(IndexFrom2D(pos, TILEMAP_WIDTH), (t_byte*)tm_activity, TILEMAP_WIDTH * TILEMAP_HEIGHT);
}

static void ActivateTile(t_tilemap_activity* const tm_activity, const s_vec_2d_i pos) {
    assert(tm_activity);
    assert(pos.x >= 0 && pos.x < TILEMAP_WIDTH && pos.y >= 0 && pos.y < TILEMAP_HEIGHT);
    ActivateBit(IndexFrom2D(pos, TILEMAP_WIDTH), (t_byte*)tm_activity, TILEMAP_WIDTH * TILEMAP_HEIGHT);
}

static s_rect_edges_i RectTilemapSpan(const s_rect rect) {
    assert(rect.width >= 0.0f && rect.height >= 0.0f);

    return RectEdgesIClamped(
        (s_rect_edges_i){
            rect.x / TILE_SIZE,
            rect.y / TILE_SIZE,
            ceilf((rect.x + rect.width) / TILE_SIZE),
            ceilf((rect.y + rect.height) / TILE_SIZE)
        },
        (s_rect_edges_i){0, 0, TILEMAP_WIDTH, TILEMAP_HEIGHT}
    );
}

static bool TileCollisionCheck(const t_tilemap_activity* const tm_activity, const s_rect collider) {
    assert(tm_activity);
    assert(collider.width > 0.0f && collider.height > 0.0f);

    const s_rect_edges_i collider_tilemap_span = RectTilemapSpan(collider);

    for (int ty = collider_tilemap_span.top; ty < collider_tilemap_span.bottom; ty++) {
        for (int tx = collider_tilemap_span.left; tx < collider_tilemap_span.right; tx++) {
            if (!IsTileActive(tm_activity, (s_vec_2d_i){tx, ty})) {
                continue;
            }

            const s_rect tile_collider = {
                TILE_SIZE * tx,
                TILE_SIZE * ty,
                TILE_SIZE,
                TILE_SIZE
            };

            if (DoRectsInters(collider, tile_collider)) {
                return true;
            }
        }
    }

    return false;
}

static void ProcTileCollisions(s_vec_2d* const vel, const s_rect collider, const t_tilemap_activity* const tm_activity) {
    assert(vel);
    assert(collider.width > 0 && collider.height > 0);
    assert(tm_activity);

    const s_rect hor_rect = RectTranslated(collider, (s_vec_2d){vel->x, 0.0f});

    if (TileCollisionCheck(tm_activity, hor_rect)) {
        vel->x = 0.0f;
    }

    const s_rect ver_rect = RectTranslated(collider, (s_vec_2d){0.0f, vel->y});

    if (TileCollisionCheck(tm_activity, ver_rect)) {
        vel->y = 0.0f;
    }

    const s_rect diag_rect = RectTranslated(collider, *vel);

    if (TileCollisionCheck(tm_activity, diag_rect)) {
        vel->x = 0.0f;
    }
}

static bool InitGame(const s_game_init_func_data* const func_data) {
    s_game* const game = func_data->user_mem;

    if (!LoadTexturesFromFiles(&game->textures, func_data->perm_mem_arena, 1, TextureIndexToFilePath)) {
        return false;
    }

    for (int ty = TILEMAP_SURFACE_LEVEL; ty < TILEMAP_HEIGHT; ty++) {
        for (int tx = 0; tx < TILEMAP_WIDTH; tx++) {
            ActivateTile(&game->tilemap_activity, (s_vec_2d_i){tx, ty});
        }
    }

    return true;
}

static s_rect PlayerCollider(const s_vec_2d player_pos) {
    const s_rect_i src_rect = g_sprite_src_rects[ek_sprite_player];
    return (s_rect){
        player_pos.x - (src_rect.width / 2.0f),
        player_pos.y - (src_rect.height / 2.0f),
        src_rect.width,
        src_rect.height,
    };
}

static bool GameTick(const s_game_tick_func_data* const func_data) {
    s_game* const game = func_data->user_mem;

    const float move_axis = IsKeyDown(ek_key_code_d, func_data->input_state) - IsKeyDown(ek_key_code_a, func_data->input_state);
    const float move_spd_dest = move_axis * PLAYER_MOVE_SPD;
    game->player_vel.x = Lerp(game->player_vel.x, move_spd_dest, PLAYER_MOVE_SPD_LERP);

    game->player_vel.y += GRAVITY;

    if (IsKeyPressed(ek_key_code_space, func_data->input_state, func_data->input_state_last)) {
        game->player_vel.y = -PLAYER_JUMP_HEIGHT;
    }

    {
        const s_rect collider = PlayerCollider(game->player_pos);
        ProcTileCollisions(&game->player_vel, collider, &game->tilemap_activity);
    }

    game->player_pos = Vec2DSum(game->player_pos, game->player_vel);

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

    {
        ZeroOut(func_data->rendering_context.state->view_mat, sizeof(func_data->rendering_context.state->view_mat));

        t_matrix_4x4* const vm = &func_data->rendering_context.state->view_mat;
        (*vm)[0][0] = VIEW_SCALE;
        (*vm)[1][1] = VIEW_SCALE;
        (*vm)[2][2] = VIEW_SCALE;
        (*vm)[3][3] = 1.0f;
    }

    // Render tilemap.
    for (int ty = 0; ty < TILEMAP_HEIGHT; ty++) {
        for (int tx = 0; tx < TILEMAP_WIDTH; tx++) {
            if (!IsTileActive(&game->tilemap_activity, (s_vec_2d_i){tx, ty})) {
                continue;
            }

            const s_vec_2d tile_world_pos = {tx * TILE_SIZE, ty * TILE_SIZE};
            RenderSprite(&func_data->rendering_context, ek_sprite_tile, &game->textures, tile_world_pos, VEC_2D_ZERO, (s_vec_2d){1.0f, 1.0f}, 0.0f, WHITE);
        }
    }

    // Render the player.
    RenderSprite(&func_data->rendering_context, ek_sprite_player, &game->textures, game->player_pos, (s_vec_2d){0.5f, 0.5f}, (s_vec_2d){1.0f, 1.0f}, 0.0f, WHITE);

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
