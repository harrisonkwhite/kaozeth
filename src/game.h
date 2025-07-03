#ifndef GAME_H
#define GAME_H

#include <zfw_game.h>
#include <zfw_utils.h>

#define GRAVITY 0.15f

#define CAMERA_SCALE 4.0f
#define UI_SCALE 2.0f

#define DMG_POPUP_TEXT_VEL_Y_MIN -4.0f
#define DMG_POPUP_TEXT_VEL_Y_MAX -2.5f
static_assert(DMG_POPUP_TEXT_VEL_Y_MIN <= DMG_POPUP_TEXT_VEL_Y_MAX, "Invalid range.");

#define NPC_LIMIT 256

#define INVENTORY_SLOT_SIZE 48.0f
#define INVENTORY_SLOT_GAP 72.0f
#define ITEM_QUANTITY_LIMIT 99 // TEMP: Will be unique per item in the future.

#define PLAYER_INVENTORY_COLUMN_CNT 8
#define PLAYER_INVENTORY_HOTBAR_LENGTH 8
static_assert(PLAYER_INVENTORY_HOTBAR_LENGTH <= 9, "Too large since each hotbar slot needs an associated digit key.");
#define PLAYER_INVENTORY_LENGTH (PLAYER_INVENTORY_COLUMN_CNT * 4)
static_assert(PLAYER_INVENTORY_LENGTH >= PLAYER_INVENTORY_COLUMN_CNT, "Player inventory needs at least one full row!");
#define PLAYER_INVENTORY_SLOT_BG_ALPHA 0.3f
#define PLAYER_INVENTORY_BG_ALPHA 0.6f
#define PLAYER_INVENTORY_HOTBAR_BOTTOM_OFFS (INVENTORY_SLOT_SIZE * 1.75f)
#define PLAYER_INVENTORY_BODY_Y_PERC 0.45f

#define POPUP_TEXT_LIMIT 1024
#define POPUP_TEXT_STR_BUF_SIZE 32
#define POPUP_TEXT_INACTIVITY_ALPHA_THRESH 0.001f
#define POPUP_TEXT_VEL_Y_MULT 0.9f
#define POPUP_TEXT_FADE_VEL_Y_ABS_THRESH 0.002f
#define POPUP_TEXT_ALPHA_MULT 0.9f

#define PROJECTILE_LIMIT 1024

#define TILE_SIZE 8
#define TILEMAP_WIDTH 120
#define TILEMAP_HEIGHT 60

#define ITEM_DROP_LIMIT 1024

#define CURSOR_HOVER_STR_BUF_SIZE 32

typedef enum {
    ek_font_eb_garamond_24,
    ek_font_eb_garamond_28,

    eks_font_cnt
} e_fonts;

typedef enum {
    ek_sprite_player,
    ek_sprite_slime,
    ek_sprite_dirt_tile,
    ek_sprite_stone_tile,
    ek_sprite_dirt_tile_item,
    ek_sprite_stone_tile_item,
    ek_sprite_pickaxe_item,
    ek_sprite_projectile,
    ek_sprite_cursor,

    eks_sprite_cnt
} e_sprite;

s_rect ColliderFromSprite(const e_sprite sprite, const s_vec_2d pos, const s_vec_2d origin);

typedef enum {
    ek_item_type_dirt_block,
    ek_item_type_stone_block,
    ek_item_type_copper_pickaxe,
    ek_item_type_wooden_sword,
    ek_item_type_wooden_bow,

    eks_item_type_cnt
} e_item_type;

typedef t_byte t_tilemap_activity[BITS_TO_BYTES(TILEMAP_HEIGHT)][BITS_TO_BYTES(TILEMAP_WIDTH)];

typedef enum {
    ek_tile_type_dirt,
    ek_tile_type_stone,

    eks_tile_type_cnt
} e_tile_type;

typedef struct {
    e_sprite spr;
    e_item_type drop_item;
} s_tile_type;

typedef struct {
    e_item_type item_type;
    int quantity;
} s_inventory_slot;

typedef enum {
    ek_npc_type_slime,

    eks_npc_type_cnt
} e_npc_type;

typedef struct {
    int jump_time;
    int jump_hor_sign;
} s_slime_npc;

typedef union {
    s_slime_npc slime;
} u_npc_type_data;

typedef struct {
    s_vec_2d pos;
    s_vec_2d vel;

    int hp;

    e_npc_type type;
    u_npc_type_data type_data;
} s_npc;

typedef t_byte t_npc_activity[BITS_TO_BYTES(NPC_LIMIT)];

typedef struct world s_world;

typedef void (*t_npc_tick_func)(s_world* const world, const int npc_index);

typedef struct {
    const char* name;

    e_sprite spr;

    t_npc_tick_func tick_func;

    int hp_max;

    int contact_dmg;
    float contact_kb;
} s_npc_type;

typedef struct {
    s_npc buf[NPC_LIMIT];
    t_npc_activity activity;
} s_npcs;

typedef enum {
    ek_projectile_type_wooden_arrow,

    eks_projectile_type_cnt
} e_projectile_type;

typedef enum {
    ek_projectile_type_flags_falls = 1 << 0,
    ek_projectile_type_flags_rot_is_dir = 1 << 1
} e_projectile_type_flags;

typedef struct {
    e_sprite spr;
    e_projectile_type_flags flags;
} s_projectile_type;

typedef struct {
    e_projectile_type type;
    bool friendly;
    int dmg;
    s_vec_2d pos;
    s_vec_2d vel;
    float rot;
} s_projectile;

typedef struct {
    e_item_type item_type;
    int quantity;
    s_vec_2d pos;
    s_vec_2d vel;
} s_item_drop;

typedef struct {
    char str[POPUP_TEXT_STR_BUF_SIZE];
    s_vec_2d pos;
    float vel_y;
    float alpha;
} s_popup_text;

typedef struct {
    t_tilemap_activity activity;
    e_tile_type tile_types[TILEMAP_HEIGHT][TILEMAP_WIDTH];
} s_tilemap;

typedef struct {
    bool killed;
    s_vec_2d pos;
    s_vec_2d vel;
    bool jumping;
    int hp;
    int invinc_time;
} s_player_ent;

typedef struct {
    int player_hp_max;
    s_tilemap tilemap;
} s_world_pers;

typedef struct world {
    s_world_pers pers;

    s_player_ent player;

    s_npcs npcs;

    s_projectile projectiles[PROJECTILE_LIMIT];
    int proj_cnt;

    s_item_drop item_drops[ITEM_DROP_LIMIT];
    int item_drop_active_cnt;

    //int tilemap_tile_lifes[TILEMAP_HEIGHT][TILEMAP_WIDTH];

    bool player_inv_open;
    s_inventory_slot player_inv_slots[PLAYER_INVENTORY_LENGTH];
    int player_inv_hotbar_slot_selected;

    s_popup_text popup_texts[POPUP_TEXT_LIMIT];

    s_vec_2d cam_pos;

    char cursor_hover_str[CURSOR_HOVER_STR_BUF_SIZE];
    e_item_type cursor_item_held_type;
    int cursor_item_held_quantity;
} s_world;

typedef enum {
    ek_item_use_type_tile_place,
    ek_item_use_type_tile_destroy,
    ek_item_use_type_shoot
} e_item_use_type;

// NOTE: Might want to partition these out into distinct arrays once more familiar with how this data will be accessed.
typedef struct {
    const char* name;
    const e_sprite spr;

    bool consume_on_use;
    e_item_use_type use_type;

    e_tile_type tile_place_type;

    e_projectile_type shoot_proj_type;
    float shoot_proj_spd;
    int shoot_proj_dmg;
} s_item_type;

extern const s_rect_i g_sprite_src_rects[];

static inline void RenderSprite(const s_rendering_context* const context, const e_sprite spr, const s_textures* const textures, const s_vec_2d pos, const s_vec_2d origin, const s_vec_2d scale, const float rot, const s_color blend) {
    RenderTexture(
        context,
        0,
        textures,
        g_sprite_src_rects[spr],
        pos,
        origin,
        scale,
        rot,
        blend
    );
}

static inline s_vec_2d_i UISize(const s_vec_2d_i display_size) {
    assert(display_size.x > 0 && display_size.y > 0);
    return Vec2DIScaled(display_size, 1.0f / UI_SCALE);
}

static inline s_vec_2d DisplayToUIPos(const s_vec_2d pos) {
    return Vec2DScaled(pos, 1.0f / UI_SCALE);
}

//
// world.c
//
bool InitWorld(s_world* const world, const char* const filename);
bool WorldTick(s_world* const world, const s_input_state* const input_state, const s_input_state* const input_state_last, const s_vec_2d_i display_size); // Returns true only if successful.
void RenderWorld(const s_rendering_context* const rendering_context, const s_world* const world, const s_textures* const textures);
bool RenderWorldUI(const s_rendering_context* const rendering_context, const s_world* const world, const s_vec_2d cursor_ui_pos, const s_textures* const textures, const s_fonts* const fonts, s_mem_arena* const temp_mem_arena);

s_popup_text* SpawnPopupText(s_world* const world, const s_vec_2d pos, const float vel_y);

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

//
// items.c
//
extern const s_item_type g_item_types[];

bool SpawnItemDrop(s_world* const world, const s_vec_2d pos, const e_item_type item_type, const int item_quantity);
void UpdateItemDrops(s_world* const world);

static inline s_rect ItemDropCollider(const s_vec_2d pos, const e_item_type item_type) {
    return ColliderFromSprite(g_item_types[item_type].spr, pos, (s_vec_2d){0.5f, 0.5f});
}

//
// player.c
//
void ProcPlayerMovement(s_world* const world, const s_input_state* const input_state, const s_input_state* const input_state_last);
bool ProcPlayerCollisionsWithNPCs(s_world* const world);
void ProcPlayerDeath(s_world* const world);
void RenderPlayer(const s_rendering_context* const rendering_context, const s_world* const world, const s_textures* const textures);
s_rect PlayerCollider(const s_vec_2d pos);
bool HurtPlayer(s_world* const world, const int dmg, const s_vec_2d kb);

//
// npcs.c
//
extern const s_npc_type g_npc_types[];

int SpawnNPC(s_npcs* const npcs, const s_vec_2d pos, const e_npc_type type); // Returns the index of the spawned NPC, or -1 if no NPC could be spawned.
void RunNPCTicks(s_world* const world);
void ProcNPCDeaths(s_world* const world);
void RenderNPCs(const s_rendering_context* const rendering_context, const s_npcs* const npcs, const s_textures* const textures);
bool HurtNPC(s_world* const world, const int npc_index, const int dmg, const s_vec_2d kb);
s_rect NPCCollider(const s_vec_2d npc_pos, const e_npc_type npc_type);
bool IsNPCActive(const t_npc_activity* const activity, const int index);

//
// projectiles.c
//
extern const s_projectile_type g_projectile_types[];

s_projectile* SpawnProjectile(s_world* const world, const e_projectile_type type, const bool friendly, const int dmg, const s_vec_2d pos, const s_vec_2d vel);
bool UpdateProjectiles(s_world* const world);
void RenderProjectiles(const s_rendering_context* const rendering_context, const s_projectile* const projectiles, const int proj_cnt, const s_textures* const textures);

//
// tiles.c
//
extern const s_tile_type g_tile_types[];

s_rect_edges_i RectTilemapSpan(const s_rect rect);
void PlaceTile(s_tilemap* const tilemap, const s_vec_2d_i pos, const e_tile_type tile_type);
void DestroyTile(s_world* const world, const s_vec_2d_i pos);
bool TileCollisionCheck(const t_tilemap_activity* const tm_activity, const s_rect collider);
void ProcTileCollisions(s_vec_2d* const vel, const s_rect collider, const t_tilemap_activity* const tm_activity);
void ProcVerTileCollisions(float* const vel_y, const s_rect collider, const t_tilemap_activity* const tm_activity);
void RenderTilemap(const s_rendering_context* const rendering_context, const s_tilemap* const tilemap, const s_rect_edges_i range, const s_textures* const textures);

static inline bool IsTilePosInBounds(const s_vec_2d_i pos) {
    return pos.x >= 0 && pos.x < TILEMAP_WIDTH && pos.y >= 0 && pos.y < TILEMAP_HEIGHT;
}

static bool IsTileActive(const t_tilemap_activity* const tm_activity, const s_vec_2d_i pos) {
    assert(tm_activity);
    assert(IsTilePosInBounds(pos));
    return IsBitActive(IndexFrom2D(pos, TILEMAP_WIDTH), (t_byte*)tm_activity, TILEMAP_WIDTH * TILEMAP_HEIGHT);
}

//
// inventory.c
//
int AddToInventory(s_inventory_slot* const slots, const int slot_cnt, const e_item_type item_type, int quantity); // Returns the quantity that couldn't be added (0 if everything was added).
int RemoveFromInventory(s_inventory_slot* const slots, const int slot_cnt, const e_item_type item_type, int quantity); // Returns the quantity that couldn't be removed (0 if everything was removed).
bool DoesInventoryHaveRoomFor(s_inventory_slot* const slots, const int slot_cnt, const e_item_type item_type, int quantity);
bool RenderInventorySlot(const s_rendering_context* const rendering_context, const s_inventory_slot slot, const s_vec_2d pos, const s_color outline_color, const s_textures* const textures, const s_fonts* const fonts, s_mem_arena* const temp_mem_arena);

void LoadPlayerInventorySlotPositions(s_vec_2d (* const positions)[PLAYER_INVENTORY_LENGTH], const s_vec_2d_i ui_size);

#endif
