#include <stdlib.h>
#include <stdio.h>
#include <zfw_game.h>
#include "game.h"
#include "items.h"
#include "zfw_math.h"

typedef enum {
    ek_font_eb_garamond_36,

    eks_font_cnt
} e_fonts;

#define GRAVITY 0.15f

#define PLAYER_MOVE_SPD 1.5f
#define PLAYER_MOVE_SPD_LERP 0.2f
#define PLAYER_JUMP_HEIGHT 3.0f

#define TILEMAP_WIDTH 120
#define TILEMAP_HEIGHT 60
#define TILE_SIZE 8
#define TILEMAP_SURFACE_LEVEL 40
static_assert(TILEMAP_SURFACE_LEVEL <= TILEMAP_HEIGHT, "Invalid tilemap surface level!");

#define VIEW_SCALE 2.0f

s_rect_i g_sprite_src_rects[eks_sprite_cnt] = {
    {1, 1, 14, 22}, // Player
    {16, 0, 8, 8}, // Tile
    {24, 0, 8, 8} // Cursor
};

typedef t_byte t_tilemap_activity[BITS_TO_BYTES(TILEMAP_HEIGHT)][BITS_TO_BYTES(TILEMAP_WIDTH)];

typedef struct {
    e_item_type item_type;
    int quantity;
} s_inventory_slot;

#define PLAYER_INVENTORY_LENGTH 32
#define INVENTORY_SLOT_SIZE 80.0f
#define INVENTORY_SLOT_GAP 112.0f
#define PLAYER_INVENTORY_POS_PERC (s_vec_2d){0.05f, 0.075f}
#define PLAYER_INVENTORY_COLUMN_CNT 8
static_assert(PLAYER_INVENTORY_COLUMN_CNT <= 9, "Player inventory column count is too large, as each hotbar slot needs an associated digit key.");
#define PLAYER_INVENTORY_SLOT_BG_ALPHA 0.3f

typedef struct {
    s_textures textures;
    s_fonts fonts;

    s_vec_2d player_pos;
    s_vec_2d player_vel;
    bool player_jumping;

    t_tilemap_activity tilemap_activity;

    bool player_inventory_open;
    s_inventory_slot player_inventory_slots[PLAYER_INVENTORY_LENGTH];
    int player_inventory_hotbar_slot_selected;

    s_vec_2d cam_pos;
} s_game;

#define ITEM_QUANTITY_LIMIT 99 // TEMP: Will be unique per item in the future.

// Returns the quantity that couldn't be added (0 if everything was added).
static int AddToInventory(s_inventory_slot* const slots, const int slot_cnt, const e_item_type item_type, int quantity) {
    assert(slots);
    assert(slot_cnt > 0);
    assert(quantity > 0);

    for (int i = 0; i < slot_cnt && quantity > 0; i++) {
        if (slots[i].quantity > 0 && slots[i].item_type == item_type && slots[i].quantity < ITEM_QUANTITY_LIMIT) {
            const int quant_to_add = MIN(ITEM_QUANTITY_LIMIT - slots[i].quantity, quantity);
            slots[i].quantity += quant_to_add;
            quantity -= quant_to_add;
        }
    }

    for (int i = 0; i < slot_cnt && quantity > 0; i++) {
        if (slots[i].quantity == 0) {
            const int quant_to_add = MIN(ITEM_QUANTITY_LIMIT, quantity);
            slots[i].quantity += quant_to_add;
            quantity -= quant_to_add;
        }
    }
    
    return quantity;
}

// Returns the quantity that couldn't be removed (0 if everything was removed).
static int RemoveFromInventory(s_inventory_slot* const slots, const int slot_cnt, const e_item_type item_type, int quantity) {
    assert(slots);
    assert(slot_cnt > 0);
    assert(quantity > 0);

    for (int i = 0; i < slot_cnt && quantity > 0; i++) {
        if (slots[i].quantity > 0 && slots[i].item_type == item_type) {
            const int quant_to_remove = MIN(slots[i].quantity, quantity);
            slots[i].quantity -= quant_to_remove;
            quantity -= quant_to_remove;
        }
    }

    return quantity;
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

static inline bool IsTilePosInBounds(const s_vec_2d_i pos) {
    return pos.x >= 0 && pos.x < TILEMAP_WIDTH && pos.y >= 0 && pos.y < TILEMAP_HEIGHT;
}

static bool IsTileActive(const t_tilemap_activity* const tm_activity, const s_vec_2d_i pos) {
    assert(tm_activity);
    assert(IsTilePosInBounds(pos));
    return IsBitActive(IndexFrom2D(pos, TILEMAP_WIDTH), (t_byte*)tm_activity, TILEMAP_WIDTH * TILEMAP_HEIGHT);
}

static void ActivateTile(t_tilemap_activity* const tm_activity, const s_vec_2d_i pos) {
    assert(tm_activity);
    assert(IsTilePosInBounds(pos));
    ActivateBit(IndexFrom2D(pos, TILEMAP_WIDTH), (t_byte*)tm_activity, TILEMAP_WIDTH * TILEMAP_HEIGHT);
}

static void DeactivateTile(t_tilemap_activity* const tm_activity, const s_vec_2d_i pos) {
    assert(tm_activity);
    assert(IsTilePosInBounds(pos));
    DeactivateBit(IndexFrom2D(pos, TILEMAP_WIDTH), (t_byte*)tm_activity, TILEMAP_WIDTH * TILEMAP_HEIGHT);
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

if (!LoadFontsFromFiles(&game->fonts, func_data->perm_mem_arena, eks_font_cnt, FontIndexToLoadInfo, func_data->temp_mem_arena)) {
        return false;
    }

    AddToInventory(game->player_inventory_slots, PLAYER_INVENTORY_LENGTH, ek_item_type_dirt_block, 2);

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

static inline s_vec_2d CameraSize(const s_vec_2d_i display_size) {
    assert(display_size.x > 0 && display_size.y > 0);
    return (s_vec_2d){display_size.x / VIEW_SCALE, display_size.y / VIEW_SCALE};
}

static inline s_vec_2d CameraTopLeft(const s_vec_2d cam_pos, const s_vec_2d_i display_size) {
    assert(display_size.x > 0 && display_size.y > 0);
    const s_vec_2d size = CameraSize(display_size);
    return (s_vec_2d){cam_pos.x - (size.x / 2.0f), cam_pos.y - (size.y / 2.0f)};
}

static inline s_vec_2d CameraToDisplayPos(const s_vec_2d pos, const s_vec_2d cam_pos, const s_vec_2d_i display_size) {
    assert(display_size.x > 0 && display_size.y > 0);
    const s_vec_2d cam_tl = CameraTopLeft(cam_pos, display_size);
    return (s_vec_2d) {
        (pos.x - cam_tl.x) * VIEW_SCALE,
        (pos.y - cam_tl.y) * VIEW_SCALE
    };
}

static inline s_vec_2d DisplayToCameraPos(const s_vec_2d pos, const s_vec_2d cam_pos, const s_vec_2d_i display_size) {
    assert(display_size.x > 0 && display_size.y > 0);
    const s_vec_2d cam_tl = CameraTopLeft(cam_pos, display_size);
    return (s_vec_2d) {
        cam_tl.x + (pos.x / VIEW_SCALE),
        cam_tl.y + (pos.y / VIEW_SCALE)
    };
}

static bool GameTick(const s_game_tick_func_data* const func_data) {
    s_game* const game = func_data->user_mem;

    //
    // Player Movement
    //
    {
        const float move_axis = IsKeyDown(ek_key_code_d, func_data->input_state) - IsKeyDown(ek_key_code_a, func_data->input_state);
        const float move_spd_dest = move_axis * PLAYER_MOVE_SPD;
        game->player_vel.x = Lerp(game->player_vel.x, move_spd_dest, PLAYER_MOVE_SPD_LERP);

        game->player_vel.y += GRAVITY;

        if (!game->player_jumping) {
            if (IsKeyPressed(ek_key_code_space, func_data->input_state, func_data->input_state_last)) {
                game->player_vel.y = -PLAYER_JUMP_HEIGHT;
                game->player_jumping = true;
            }
        } else {
            if (game->player_vel.y < 0.0f && !IsKeyDown(ek_key_code_space, func_data->input_state)) {
                game->player_vel.y = 0.0f;
            }
        }

        {
            const s_rect collider = PlayerCollider(game->player_pos);
            ProcTileCollisions(&game->player_vel, collider, &game->tilemap_activity);
        }

        game->player_pos = Vec2DSum(game->player_pos, game->player_vel);

        // Leave jumping state if tile is below.
        const s_rect below_collider = RectTranslated(PlayerCollider(game->player_pos), (s_vec_2d){0.0f, 1.0f});

        if (TileCollisionCheck(&game->tilemap_activity, below_collider)) {
            game->player_jumping = false;
        }
    }

    //
    // Camera
    //
    game->cam_pos = game->player_pos;

    //
    // Tilemap Interaction
    //
    if (IsMouseButtonPressed(ek_mouse_button_code_left, func_data->input_state, func_data->input_state_last)) {
        const s_vec_2d mouse_cam_pos = DisplayToCameraPos(func_data->input_state->mouse_pos, game->cam_pos, func_data->window_state.size);

        const s_vec_2d_i mouse_tile_pos = {
            floorf(mouse_cam_pos.x / TILE_SIZE),
            floorf(mouse_cam_pos.y / TILE_SIZE)
        };

        if (IsTilePosInBounds(mouse_tile_pos)) {
            DeactivateTile(&game->tilemap_activity, mouse_tile_pos);
        }
    }

    //
    // Player Inventory
    //
    if (IsKeyPressed(ek_key_code_escape, func_data->input_state, func_data->input_state_last)) {
        game->player_inventory_open = !game->player_inventory_open;
    }

    for (int i = 0; i < PLAYER_INVENTORY_COLUMN_CNT; i++) {
        if (IsKeyPressed(ek_key_code_1 + i, func_data->input_state, func_data->input_state_last)) {
            game->player_inventory_hotbar_slot_selected = i;
            break;
        }
    }

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

static void InitCameraViewMatrix4x4(t_matrix_4x4* const mat, const s_vec_2d cam_pos, const s_vec_2d_i display_size) {
    assert(mat && IsZero(mat, sizeof(*mat)));
    assert(display_size.x > 0 && display_size.y > 0);

    const s_vec_2d view_pos = {
        (-cam_pos.x * VIEW_SCALE) + (display_size.x / 2.0f),
        (-cam_pos.y * VIEW_SCALE) + (display_size.y / 2.0f)
    };

    InitIdenMatrix4x4(mat);
    TranslateMatrix4x4(mat, view_pos);
    ScaleMatrix4x4(mat, VIEW_SCALE);
}

static bool RenderGame(const s_game_render_func_data* const func_data) {
    s_game* const game = func_data->user_mem;

    RenderClear((s_color){0.2, 0.3, 0.4, 1.0});

    //
    // World
    //
    ZeroOut(func_data->rendering_context.state->view_mat, sizeof(func_data->rendering_context.state->view_mat));
    InitCameraViewMatrix4x4(&func_data->rendering_context.state->view_mat, game->cam_pos, func_data->rendering_context.display_size);

    // Render tilemap.
    for (int ty = 0; ty < TILEMAP_HEIGHT; ty++) {
        for (int tx = 0; tx < TILEMAP_WIDTH; tx++) {
            if (!IsTileActive(&game->tilemap_activity, (s_vec_2d_i){tx, ty})) {
                continue;
            }

            const s_vec_2d tile_world_pos = {tx * TILE_SIZE, ty * TILE_SIZE};
            RenderSprite(&func_data->rendering_context, ek_sprite_dirt_tile, &game->textures, tile_world_pos, VEC_2D_ZERO, (s_vec_2d){1.0f, 1.0f}, 0.0f, WHITE);
        }
    }

    // Render the player.
    RenderSprite(&func_data->rendering_context, ek_sprite_player, &game->textures, game->player_pos, (s_vec_2d){0.5f, 0.5f}, (s_vec_2d){1.0f, 1.0f}, 0.0f, WHITE);

    Flush(&func_data->rendering_context);

    //
    // UI
    //

    ZeroOut(func_data->rendering_context.state->view_mat, sizeof(func_data->rendering_context.state->view_mat));
    InitIdenMatrix4x4(&func_data->rendering_context.state->view_mat);

    // Render the player inventory.
    const s_vec_2d player_inv_pos = {
        func_data->rendering_context.display_size.x * PLAYER_INVENTORY_POS_PERC.x,
        func_data->rendering_context.display_size.y * PLAYER_INVENTORY_POS_PERC.y
    };

    const int slot_display_cnt = game->player_inventory_open ? PLAYER_INVENTORY_LENGTH : PLAYER_INVENTORY_COLUMN_CNT;

    for (int i = 0; i < slot_display_cnt; i++) {
        const s_inventory_slot* const slot = &game->player_inventory_slots[i];

        const s_rect slot_rect = {
            player_inv_pos.x + (INVENTORY_SLOT_GAP * (i % PLAYER_INVENTORY_COLUMN_CNT)),
            player_inv_pos.y + ((int)(i / PLAYER_INVENTORY_COLUMN_CNT) * INVENTORY_SLOT_GAP),
            INVENTORY_SLOT_SIZE,
            INVENTORY_SLOT_SIZE
        };

        const s_color slot_outline_color = game->player_inventory_hotbar_slot_selected == i ? YELLOW : WHITE;

        // Render the slot box.
        RenderRect(&func_data->rendering_context, slot_rect, (s_color){0.0f, 0.0f, 0.0f, PLAYER_INVENTORY_SLOT_BG_ALPHA});
        RenderRectOutline(&func_data->rendering_context, slot_rect, slot_outline_color, VIEW_SCALE);

        // Render the item icon.
        if (slot->quantity > 0) {
            RenderSprite(&func_data->rendering_context, g_items[slot->item_type].sprite, &game->textures, RectCenter(slot_rect), (s_vec_2d){0.5f, 0.5f}, (s_vec_2d){VIEW_SCALE, VIEW_SCALE}, 0.0f, WHITE);
        }

        // Render the quantity.
        if (slot->quantity > 1) {
            char quant_str_buf[4];
            snprintf(quant_str_buf, sizeof(quant_str_buf), "%d", slot->quantity);

            const s_vec_2d quant_pos = {
                slot_rect.x + slot_rect.width - 14.0f,
                slot_rect.y + slot_rect.height - 6.0f
            };

            RenderStr(&func_data->rendering_context, quant_str_buf, ek_font_eb_garamond_36, &game->fonts, quant_pos, ek_str_hor_align_right, ek_str_ver_align_bottom, WHITE, func_data->temp_mem_arena);
        }
    }

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
