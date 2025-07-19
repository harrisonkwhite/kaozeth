#ifndef WORLD_H
#define WORLD_H

#include "../game.h"
#include "../lighting.h"
#include "../inventory.h"
#include "../particles.h"

typedef struct world s_world;

#define DEATH_TEXT "You were slain..."

#define GRAVITY 0.2f

#define CAMERA_LERP 0.3f
#define CAMERA_SCALE 2.0f

#define POPUP_TEXT_LIMIT 1024
#define POPUP_TEXT_STR_BUF_SIZE 32
#define POPUP_TEXT_INACTIVITY_ALPHA_THRESH 0.001f
#define POPUP_TEXT_VEL_Y_MULT 0.9f
#define POPUP_TEXT_FADE_VEL_Y_ABS_THRESH 0.002f
#define POPUP_TEXT_ALPHA_MULT 0.9f

#define TILE_SIZE 8
#define TILE_PLACE_DIST 4
#define TILE_HIGHLIGHT_ALPHA 0.4f
#define TILEMAP_WIDTH 640 // TEMP: Make dynamic!
#define TILEMAP_HEIGHT 200 // TEMP: Make dynamic!

#define WORLD_WIDTH (TILEMAP_WIDTH * TILE_SIZE)
#define WORLD_HEIGHT (TILEMAP_HEIGHT * TILE_SIZE)

#define WORLD_NAME_LEN_LIMIT 20
#define WORLD_FILENAME_EXT ".wrld"
#define WORLD_FILENAME_BUF_SIZE (WORLD_NAME_LEN_LIMIT + sizeof(WORLD_FILENAME_EXT))

typedef char t_world_filename[WORLD_FILENAME_BUF_SIZE];

typedef struct {
    char str[POPUP_TEXT_STR_BUF_SIZE];
    s_vec_2d pos;
    float vel_y;
    float alpha;
} s_popup_text;

//
// UI
//
#define PLAYER_INVENTORY_COLUMN_CNT 6
static_assert(PLAYER_INVENTORY_COLUMN_CNT <= 9, "Too large since each hotbar slot needs an associated digit key.");
#define PLAYER_INVENTORY_ROW_CNT 3
#define PLAYER_INVENTORY_LEN (PLAYER_INVENTORY_COLUMN_CNT * PLAYER_INVENTORY_ROW_CNT)

#define DMG_POPUP_TEXT_VEL_Y_MIN -4.0f
#define DMG_POPUP_TEXT_VEL_Y_MAX -2.5f
static_assert(DMG_POPUP_TEXT_VEL_Y_MIN <= DMG_POPUP_TEXT_VEL_Y_MAX, "Invalid range.");

#define MOUSE_HOVER_STR_BUF_SIZE 32

typedef char t_mouse_hover_str_buf[MOUSE_HOVER_STR_BUF_SIZE];

//
// Player
//
#define PLAYER_INIT_HP_MAX 100

typedef struct {
    bool killed;
    s_vec_2d pos;
    s_vec_2d vel;
    bool jumping;
    int hp;
    int invinc_time;
    int item_use_break;
} s_player;

//
// NPCs
//
#define NPC_LIMIT 256

typedef t_byte t_npc_activity[BITS_TO_BYTES(NPC_LIMIT)];

typedef void (*t_npc_tick_func)(s_world* const world, const int npc_index);
typedef void (*t_npc_postspawn_func)(s_world* const world, const int npc_index);

typedef enum {
    ek_npc_type_slime,

    eks_npc_type_cnt
} e_npc_type;

typedef struct {
    const char* name;

    e_sprite spr;

    t_npc_tick_func tick_func;
    t_npc_postspawn_func postspawn_func;

    int hp_max;

    int contact_dmg;
    float contact_kb;
} s_npc_type;

typedef struct {
    int jump_time;
    float jump_hor_spd;
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

typedef struct {
    s_npc buf[NPC_LIMIT];
    t_npc_activity activity;
} s_npcs;

//
// Projectiles
//
#define PROJECTILE_LIMIT 1024

typedef struct {
    e_projectile_type type;
    bool friendly;
    int dmg;
    s_vec_2d pos;
    s_vec_2d vel;
    float rot;
} s_projectile;

//
// Tilemap
//
typedef t_byte t_tilemap_activity[BITS_TO_BYTES(TILEMAP_HEIGHT)][BITS_TO_BYTES(TILEMAP_WIDTH)];

typedef int t_tilemap_tile_lifes[TILEMAP_HEIGHT][TILEMAP_WIDTH];

typedef struct {
    t_tilemap_activity activity;
    e_tile_type tile_types[TILEMAP_HEIGHT][TILEMAP_WIDTH];
} s_tilemap_core;

//
// Items
//
#define ITEM_DROP_LIMIT 1024
#define ITEM_DROP_ORIGIN (s_vec_2d){0.5f, 0.5f}

typedef struct {
    e_item_type item_type;
    int quantity;
    s_vec_2d pos;
    s_vec_2d vel;
} s_item_drop;

//
//
//
#define WORLD_MEM_ARENA_SIZE ((1 << 20) * 2)

typedef struct {
    int player_hp_max;
    s_tilemap_core tilemap_core;
} s_world_core;

typedef struct world {
    s_mem_arena mem_arena;

    s_world_core core;

    s_lightmap lightmap;

    int respawn_time;

    s_player player;

    s_npcs npcs;

    s_projectile projectiles[PROJECTILE_LIMIT];
    int proj_cnt;

    s_item_drop item_drops[ITEM_DROP_LIMIT];
    int item_drop_active_cnt;

    t_tilemap_tile_lifes tilemap_tile_lifes;

    bool player_inv_open;
    s_inventory_slot player_inv_slots[PLAYER_INVENTORY_ROW_CNT][PLAYER_INVENTORY_COLUMN_CNT];
    int player_inv_hotbar_slot_selected;

    s_popup_text popup_texts[POPUP_TEXT_LIMIT];

    s_vec_2d cam_pos;

    t_mouse_hover_str_buf mouse_hover_str;
    e_item_type mouse_item_held_type;
    int mouse_item_held_quantity;

    s_particles particles;
} s_world;

static inline s_vec_2d_i CameraToTilePos(const s_vec_2d pos) {
    return (s_vec_2d_i){
        floorf(pos.x / TILE_SIZE),
        floorf(pos.y / TILE_SIZE)
    };
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

static inline s_vec_2d CameraToUIPos(const s_vec_2d pos, const s_vec_2d cam_pos, const s_vec_2d_i display_size) {
    return DisplayToUIPos(CameraToDisplayPos(pos, cam_pos, display_size));
}

//
// world.c
//
bool InitWorld(s_world* const world, const t_world_filename* const filename, s_mem_arena* const temp_mem_arena);
void CleanWorld(s_world* const world);
bool WorldTick(s_world* const world, const t_settings* const settings, const s_input_state* const input_state, const s_input_state* const input_state_last, const s_vec_2d_i display_size, s_audio_sys* const audio_sys, const s_sound_types* const snd_types);
void RenderWorld(const s_rendering_context* const rendering_context, const s_world* const world, const s_textures* const textures);
bool LoadWorldCoreFromFile(s_world_core* const world_core, const t_world_filename* const filename);
bool WriteWorldCoreToFile(const s_world_core* const world_core, const t_world_filename* const filename);

s_popup_text* SpawnPopupText(s_world* const world, const s_vec_2d pos, const float vel_y);

//
// world_ui.c
//
void UpdateWorldUI(s_world* const world, const s_input_state* const input_state, const s_input_state* const input_state_last, const s_vec_2d_i display_size);
bool RenderWorldUI(const s_rendering_context* const rendering_context, const s_world* const world, const s_vec_2d mouse_pos, const s_textures* const textures, const s_fonts* const fonts, s_mem_arena* const temp_mem_arena);

//
// world_gen.c
//
void GenWorld(s_world_core* const world_core);

//
// player.c
//
void InitPlayer(s_player* const player, const int hp_max, const t_tilemap_activity* const tm_activity);
void ProcPlayerMovement(s_world* const world, const s_input_state* const input_state, const s_input_state* const input_state_last);
bool ProcPlayerCollisionsWithNPCs(s_world* const world);
void ProcPlayerDeath(s_world* const world);
void RenderPlayer(const s_rendering_context* const rendering_context, const s_world* const world, const s_textures* const textures);
bool HurtPlayer(s_world* const world, const int dmg, const s_vec_2d kb);

#define PLAYER_ORIGIN (s_vec_2d){0.5f, 0.5f}

static inline s_vec_2d PlayerColliderSize() {
    const s_sprite* const spr = &g_sprites[ek_sprite_player];
    return (s_vec_2d){spr->src_rect.width, spr->src_rect.height};
}

static inline s_rect PlayerCollider(const s_vec_2d pos) {
    return Collider(pos, PlayerColliderSize(), PLAYER_ORIGIN);
}

//
// npcs.c
//
#define NPC_ORIGIN (s_vec_2d){0.5f, 0.5f}

extern const s_npc_type g_npc_types[];

int SpawnNPC(s_world* const world, const s_vec_2d pos, const e_npc_type type, const t_tilemap_activity* const tm_activity); // Returns the index of the spawned NPC, or -1 if no NPC could be spawned.
void UpdateNPCs(s_world* const world);
void ProcNPCDeaths(s_world* const world);
void RenderNPCs(const s_rendering_context* const rendering_context, const s_npcs* const npcs, const s_textures* const textures);
bool HurtNPC(s_world* const world, const int npc_index, const int dmg, const s_vec_2d kb);
bool IsNPCActive(const t_npc_activity* const activity, const int index);
bool ProcEnemySpawning(s_world* const world, const float cam_width);

static inline s_vec_2d NPCColliderSize(const e_npc_type npc_type) {
    const s_sprite* const spr = &g_sprites[g_npc_types[npc_type].spr];
    return (s_vec_2d){spr->src_rect.width, spr->src_rect.height};
}

static inline s_rect NPCCollider(const s_vec_2d npc_pos, const e_npc_type npc_type) {
    return Collider(npc_pos, NPCColliderSize(npc_type), NPC_ORIGIN);
}

//
// projectiles.c
//
s_projectile* SpawnProjectile(s_world* const world, const e_projectile_type type, const bool friendly, const int dmg, const s_vec_2d pos, const s_vec_2d vel);
bool UpdateProjectiles(s_world* const world);
void RenderProjectiles(const s_rendering_context* const rendering_context, const s_projectile* const projectiles, const int proj_cnt, const s_textures* const textures);

static inline s_rect ProjectileCollider(const e_projectile_type proj_type, const s_vec_2d pos) {
    return ColliderFromSprite(g_projectile_types[proj_type].spr, pos, (s_vec_2d){0.5f, 0.5f});
}

//
// tiles.c
//
s_rect_edges_i RectTilemapSpan(const s_rect rect);
void PlaceTile(s_tilemap_core* const tilemap, const s_vec_2d_i pos, const e_tile_type tile_type);
void HurtTile(s_world* const world, const s_vec_2d_i pos);
void DestroyTile(s_world* const world, const s_vec_2d_i pos);
bool IsTilePosFree(const s_world* const world, const s_vec_2d_i tile_pos);
bool TileCollisionCheck(const t_tilemap_activity* const tm_activity, const s_rect collider);
void ProcTileCollisions(s_vec_2d* const pos, s_vec_2d* const vel, const s_vec_2d collider_size, const s_vec_2d collider_origin, const t_tilemap_activity* const tm_activity);
void ProcVerTileCollisions(s_vec_2d* const pos, float* const vel_y, const s_vec_2d collider_size, const s_vec_2d collider_origin, const t_tilemap_activity* const tm_activity);
void MakeContactWithTilemap(s_vec_2d* const pos, const e_cardinal_dir dir, const s_vec_2d collider_size, const s_vec_2d collider_origin, const t_tilemap_activity* const tm_activity);
void MakeContactWithTilemapByJumpSize(s_vec_2d* const pos, const float jump_size, const e_cardinal_dir dir, const s_vec_2d collider_size, const s_vec_2d collider_origin, const t_tilemap_activity* const tm_activity);
s_rect_edges_i TilemapRenderRange(const s_vec_2d cam_pos, const s_vec_2d_i display_size);
void RenderTilemap(const s_rendering_context* const rendering_context, const s_tilemap_core* const tilemap_core, const t_tilemap_tile_lifes* const tilemap_tile_lifes, const s_rect_edges_i range, const s_textures* const textures);

static inline bool IsTilePosInBounds(const s_vec_2d_i pos) {
    return pos.x >= 0 && pos.x < TILEMAP_WIDTH && pos.y >= 0 && pos.y < TILEMAP_HEIGHT;
}

static inline bool IsTilemapRangeValid(const s_rect_edges_i range) {
    return IsRangeValid(range, (s_vec_2d_i){TILEMAP_WIDTH, TILEMAP_HEIGHT});
}

static inline int TileDist(const s_vec_2d_i a, const s_vec_2d_i b) {
    return Dist((s_vec_2d){a.x, a.y}, (s_vec_2d){b.x, b.y});
}

static bool IsTileActive(const t_tilemap_activity* const tm_activity, const s_vec_2d_i pos) {
    assert(tm_activity);
    assert(IsTilePosInBounds(pos));
    return IsBitActive(IndexFrom2D(pos, TILEMAP_WIDTH), (t_byte*)tm_activity, TILEMAP_WIDTH * TILEMAP_HEIGHT);
}

//
// items.c
//
bool IsItemUsable(const e_item_type item_type, const s_world* const world, const s_vec_2d_i mouse_tile_pos);
bool ProcItemUsage(s_world* const world, const s_input_state* const input_state, const s_vec_2d_i display_size);
bool SpawnItemDrop(s_world* const world, const s_vec_2d pos, const e_item_type item_type, const int item_quantity);
bool UpdateItemDrops(s_world* const world, s_audio_sys* const audio_sys, const s_sound_types* const snd_types, const t_settings* const settings);
void RenderItemDrops(const s_rendering_context* const rendering_context, const s_item_drop* const drops, const int drop_cnt, const s_textures* const textures);

static inline s_vec_2d ItemDropColliderSize(const e_item_type item_type) {
    const s_sprite* const spr = &g_sprites[g_item_types[item_type].icon_spr];
    return (s_vec_2d){spr->src_rect.width, spr->src_rect.height};
}

static inline s_rect ItemDropCollider(const s_vec_2d pos, const e_item_type item_type) {
    return Collider(pos, ItemDropColliderSize(item_type), ITEM_DROP_ORIGIN);
}

#endif
