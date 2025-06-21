#ifndef GAME_H
#define GAME_H

#include <zfw_game.h>

#define GRAVITY 0.15f

#define PLAYER_MOVE_SPD 1.5f
#define PLAYER_MOVE_SPD_LERP 0.2f
#define PLAYER_JUMP_HEIGHT 3.0f

#define CAMERA_SCALE 2.0f

#define INVENTORY_SLOT_SIZE 48.0f
#define INVENTORY_SLOT_GAP 72.0f
#define ITEM_QUANTITY_LIMIT 99 // TEMP: Will be unique per item in the future.

#define PLAYER_INVENTORY_LENGTH 32
#define PLAYER_INVENTORY_POS_PERC (s_vec_2d){0.05f, 0.075f}
#define PLAYER_INVENTORY_COLUMN_CNT 8
static_assert(PLAYER_INVENTORY_COLUMN_CNT <= 9, "Player inventory column count is too large, as each hotbar slot needs an associated digit key.");
#define PLAYER_INVENTORY_SLOT_BG_ALPHA 0.3f

typedef enum {
    ek_font_eb_garamond_36,

    eks_font_cnt
} e_fonts;

typedef enum {
    ek_sprite_player,
    ek_sprite_slime,
    ek_sprite_dirt_tile,
    ek_sprite_cursor,

    eks_sprite_cnt
} e_sprite;

#define TILE_SIZE 8
#define TILEMAP_WIDTH 120
#define TILEMAP_HEIGHT 60

typedef t_byte t_tilemap_activity[BITS_TO_BYTES(TILEMAP_HEIGHT)][BITS_TO_BYTES(TILEMAP_WIDTH)];

typedef enum {
    ek_item_type_dirt_block,
    ek_item_type_wooden_sword,

    eks_item_type_cnt
} e_item_type;

typedef struct {
    e_item_type item_type;
    int quantity;
} s_inventory_slot;

typedef struct {
    s_vec_2d player_pos;
    s_vec_2d player_vel;
    bool player_jumping;

    t_tilemap_activity tilemap_activity;

    bool player_inventory_open;
    s_inventory_slot player_inventory_slots[PLAYER_INVENTORY_LENGTH];
    int player_inventory_hotbar_slot_selected;

    s_vec_2d cam_pos;
} s_world;

typedef enum {
    ek_npc_slime
} e_npc;

// NOTE: Might want to partition these out into distinct arrays once more familiar with how this data will be accessed.
typedef struct {
    const char* name;
    const e_sprite sprite;
} s_item;

extern const s_item g_items[eks_item_type_cnt];

extern const s_rect_i g_sprite_src_rects[eks_sprite_cnt];

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

void InitWorld(s_world* const world);
void WorldTick(s_world* const world, const s_input_state* const input_state, const s_input_state* const input_state_last, const s_vec_2d_i display_size);
void RenderWorld(const s_rendering_context* const rendering_context, const s_world* const world, const s_textures* const textures, const s_fonts* const fonts, s_mem_arena* const temp_mem_arena);

s_rect_edges_i RectTilemapSpan(const s_rect rect);
bool TileCollisionCheck(const t_tilemap_activity* const tm_activity, const s_rect collider);
void ProcTileCollisions(s_vec_2d* const vel, const s_rect collider, const t_tilemap_activity* const tm_activity);
void RenderTilemap(const s_rendering_context* const rendering_context, const t_tilemap_activity* const tm_activity, const s_rect_edges_i range, const s_textures* const textures);

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

static inline s_vec_2d CameraSize(const s_vec_2d_i display_size) {
    assert(display_size.x > 0 && display_size.y > 0);
    return (s_vec_2d){display_size.x / CAMERA_SCALE, display_size.y / CAMERA_SCALE};
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
        (pos.x - cam_tl.x) * CAMERA_SCALE,
        (pos.y - cam_tl.y) * CAMERA_SCALE
    };
}

static inline s_vec_2d DisplayToCameraPos(const s_vec_2d pos, const s_vec_2d cam_pos, const s_vec_2d_i display_size) {
    assert(display_size.x > 0 && display_size.y > 0);
    const s_vec_2d cam_tl = CameraTopLeft(cam_pos, display_size);
    return (s_vec_2d) {
        cam_tl.x + (pos.x / CAMERA_SCALE),
        cam_tl.y + (pos.y / CAMERA_SCALE)
    };
}

#endif
