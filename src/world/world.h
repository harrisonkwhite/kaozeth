#ifndef WORLD_H
#define WORLD_H

#include <zfw.h>
#include "../game.h"
#include "../inventory.h"
#include "../particles.h"
#include "../tilemap.h"

typedef struct world s_world;

#define DEATH_TEXT "You were slain..."

#define GRAVITY 0.2f

#define CAMERA_LERP 0.3f

#define POPUP_TEXT_LIMIT 1024
#define POPUP_TEXT_STR_BUF_SIZE 32
#define POPUP_TEXT_INACTIVITY_ALPHA_THRESH 0.001f
#define POPUP_TEXT_VEL_Y_MULT 0.9f
#define POPUP_TEXT_FADE_VEL_Y_ABS_THRESH 0.002f
#define POPUP_TEXT_ALPHA_MULT 0.9f

#define TILE_PLACE_DIST 4
#define TILE_HIGHLIGHT_ALPHA 0.4f

#define WORLD_WIDTH (TILEMAP_WIDTH * TILE_SIZE)
#define WORLD_HEIGHT (TILEMAP_HEIGHT * TILE_SIZE)

#define WORLD_NAME_LEN_LIMIT 20
#define WORLD_FILENAME_EXT ".wrld"
#define WORLD_FILENAME_BUF_SIZE (WORLD_NAME_LEN_LIMIT + sizeof(WORLD_FILENAME_EXT))

typedef char t_world_filename[WORLD_FILENAME_BUF_SIZE];

typedef struct {
    char str[POPUP_TEXT_STR_BUF_SIZE];
    zfw_s_vec_2d pos;
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
    zfw_s_vec_2d pos;
    zfw_s_vec_2d vel;
    bool jumping;
    int hp;
    int invinc_time;
    int item_use_break;
} s_player;

//
// NPCs
//
#define NPC_LIMIT 256

typedef t_u8 t_npc_activity[BITS_TO_BYTES(NPC_LIMIT)];

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
    zfw_s_vec_2d pos;
    zfw_s_vec_2d vel;

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
    zfw_s_vec_2d pos;
    zfw_s_vec_2d vel;
    float rot;
} s_projectile;

//
// Items
//
#define ITEM_DROP_LIMIT 1024
#define ITEM_DROP_ORIGIN (zfw_s_vec_2d){0.5f, 0.5f}

typedef struct {
    e_item_type item_type;
    int quantity;
    zfw_s_vec_2d pos;
    zfw_s_vec_2d vel;
} s_item_drop;

//
// Camera
//
typedef struct {
    zfw_s_vec_2d pos;
    float scale;
} s_camera;

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

    s_camera cam;

    t_mouse_hover_str_buf mouse_hover_str;
    e_item_type mouse_item_held_type;
    int mouse_item_held_quantity;

    s_particles particles;
} s_world;

static inline zfw_s_vec_2d_s32 CameraToTilePos(const zfw_s_vec_2d pos) {
    return (zfw_s_vec_2d_s32){
        floorf(pos.x / TILE_SIZE),
        floorf(pos.y / TILE_SIZE)
    };
}

static inline zfw_s_vec_2d CameraSize(const float cam_scale, const zfw_s_vec_2d_s32 window_size) {
    assert(cam_scale > 0.0f);
    assert(window_size.x > 0 && window_size.y > 0);
    return (zfw_s_vec_2d){window_size.x / cam_scale, window_size.y / cam_scale};
}

static inline zfw_s_vec_2d CameraTopLeft(const s_camera* const cam, const zfw_s_vec_2d_s32 window_size) {
    assert(window_size.x > 0 && window_size.y > 0);

    const zfw_s_vec_2d size = CameraSize(cam->scale, window_size);
    return (zfw_s_vec_2d){cam->pos.x - (size.x / 2.0f), cam->pos.y - (size.y / 2.0f)};
}

static inline zfw_s_vec_2d CameraToDisplayPos(const zfw_s_vec_2d pos, const s_camera* const cam, const zfw_s_vec_2d_s32 window_size) {
    assert(window_size.x > 0 && window_size.y > 0);

    const zfw_s_vec_2d cam_tl = CameraTopLeft(cam, window_size);
    return (zfw_s_vec_2d) {
        (pos.x - cam_tl.x) * cam->scale,
        (pos.y - cam_tl.y) * cam->scale
    };
}

static inline zfw_s_vec_2d DisplayToCameraPos(const zfw_s_vec_2d pos, const s_camera* const cam, const zfw_s_vec_2d_s32 window_size) {
    assert(window_size.x > 0 && window_size.y > 0);

    const zfw_s_vec_2d cam_tl = CameraTopLeft(cam, window_size);
    return (zfw_s_vec_2d) {
        cam_tl.x + (pos.x / cam->scale),
        cam_tl.y + (pos.y / cam->scale)
    };
}

static inline zfw_s_vec_2d CameraToUIPos(const zfw_s_vec_2d pos, const s_camera* const cam, const zfw_s_vec_2d_s32 window_size) {
    assert(window_size.x > 0 && window_size.y > 0);
    return DisplayToUIPos(CameraToDisplayPos(pos, cam, window_size));
}

//
// world.c
//
bool InitWorld(s_world* const world, const t_world_filename* const filename, const zfw_s_vec_2d_s32 window_size, s_mem_arena* const temp_mem_arena);
void CleanWorld(s_world* const world);
bool WorldTick(s_world* const world, const t_settings* const settings, const zfw_s_input_state* const input_state, const zfw_s_input_state* const input_state_last, const zfw_s_vec_2d_s32 window_size, zfw_s_audio_sys* const audio_sys, const zfw_s_sound_types* const snd_types);
bool RenderWorld(const zfw_s_rendering_context* const rendering_context, const s_world* const world, const zfw_s_textures* const textures, s_mem_arena* const temp_mem_arena);
bool LoadWorldCoreFromFile(s_world_core* const world_core, const t_world_filename* const filename);
bool WriteWorldCoreToFile(const s_world_core* const world_core, const t_world_filename* const filename);
bool PlaceWorldTile(s_world* const world, const zfw_s_vec_2d_s32 pos, const e_tile_type type);
bool HurtWorldTile(s_world* const world, const zfw_s_vec_2d_s32 pos);
bool DestroyWorldTile(s_world* const world, const zfw_s_vec_2d_s32 pos);
bool IsTilePosFree(const s_world* const world, const zfw_s_vec_2d_s32 tile_pos);
s_popup_text* SpawnPopupText(s_world* const world, const zfw_s_vec_2d pos, const float vel_y);

//
// world_ui.c
//
void UpdateWorldUI(s_world* const world, const zfw_s_input_state* const input_state, const zfw_s_input_state* const input_state_last, const zfw_s_vec_2d_s32 window_size);
bool RenderWorldUI(const zfw_s_rendering_context* const rendering_context, const s_world* const world, const zfw_s_vec_2d mouse_pos, const zfw_s_textures* const textures, const zfw_s_fonts* const fonts, s_mem_arena* const temp_mem_arena);

//
// world_gen.c
//
void GenWorld(s_world_core* const world_core);

//
// player.c
//
void InitPlayer(s_player* const player, const int hp_max, const t_tilemap_activity* const tm_activity);
void ProcPlayerMovement(s_world* const world, const zfw_s_input_state* const input_state, const zfw_s_input_state* const input_state_last);
bool ProcPlayerCollisionsWithNPCs(s_world* const world);
void ProcPlayerDeath(s_world* const world);
void RenderPlayer(const zfw_s_rendering_context* const rendering_context, const s_world* const world, const zfw_s_textures* const textures);
bool HurtPlayer(s_world* const world, const int dmg, const zfw_s_vec_2d kb); // Returns true if successful, false otherwise.

#define PLAYER_ORIGIN (zfw_s_vec_2d){0.5f, 0.5f}

static inline zfw_s_vec_2d PlayerColliderSize() {
    const s_sprite* const spr = &g_sprites[ek_sprite_player];
    return (zfw_s_vec_2d){spr->src_rect.width, spr->src_rect.height};
}

static inline zfw_s_rect PlayerCollider(const zfw_s_vec_2d pos) {
    return Collider(pos, PlayerColliderSize(), PLAYER_ORIGIN);
}

//
// npcs.c
//
#define NPC_ORIGIN (zfw_s_vec_2d){0.5f, 0.5f}

extern const s_npc_type g_npc_types[];

int SpawnNPC(s_world* const world, const zfw_s_vec_2d pos, const e_npc_type type, const t_tilemap_activity* const tm_activity); // Returns the index of the spawned NPC, or -1 if no NPC could be spawned.
void UpdateNPCs(s_world* const world);
void ProcNPCDeaths(s_world* const world);
void RenderNPCs(const zfw_s_rendering_context* const rendering_context, const s_npcs* const npcs, const zfw_s_textures* const textures);
bool HurtNPC(s_world* const world, const int npc_index, const int dmg, const zfw_s_vec_2d kb);
bool IsNPCActive(const t_npc_activity* const activity, const int index);
bool ProcEnemySpawning(s_world* const world, const float cam_width);

static inline zfw_s_vec_2d NPCColliderSize(const e_npc_type npc_type) {
    const s_sprite* const spr = &g_sprites[g_npc_types[npc_type].spr];
    return (zfw_s_vec_2d){spr->src_rect.width, spr->src_rect.height};
}

static inline zfw_s_rect NPCCollider(const zfw_s_vec_2d npc_pos, const e_npc_type npc_type) {
    return Collider(npc_pos, NPCColliderSize(npc_type), NPC_ORIGIN);
}

//
// projectiles.c
//
s_projectile* SpawnProjectile(s_world* const world, const e_projectile_type type, const bool friendly, const int dmg, const zfw_s_vec_2d pos, const zfw_s_vec_2d vel);
bool UpdateProjectiles(s_world* const world);
void RenderProjectiles(const zfw_s_rendering_context* const rendering_context, const s_projectile* const projectiles, const int proj_cnt, const zfw_s_textures* const textures);

static inline zfw_s_rect ProjectileCollider(const e_projectile_type proj_type, const zfw_s_vec_2d pos) {
    return ColliderFromSprite(g_projectile_types[proj_type].spr, pos, (zfw_s_vec_2d){0.5f, 0.5f});
}

//
// items.c
//
bool IsItemUsable(const e_item_type item_type, const s_world* const world, const zfw_s_vec_2d_s32 mouse_tile_pos);
bool ProcItemUsage(s_world* const world, const zfw_s_input_state* const input_state, const zfw_s_vec_2d_s32 window_size);
bool SpawnItemDrop(s_world* const world, const zfw_s_vec_2d pos, const e_item_type item_type, const int item_quantity);
bool UpdateItemDrops(s_world* const world, zfw_s_audio_sys* const audio_sys, const zfw_s_sound_types* const snd_types, const t_settings* const settings);
void RenderItemDrops(const zfw_s_rendering_context* const rendering_context, const s_item_drop* const drops, const int drop_cnt, const zfw_s_textures* const textures);

static inline zfw_s_vec_2d ItemDropColliderSize(const e_item_type item_type) {
    const s_sprite* const spr = &g_sprites[g_item_types[item_type].icon_spr];
    return (zfw_s_vec_2d){spr->src_rect.width, spr->src_rect.height};
}

static inline zfw_s_rect ItemDropCollider(const zfw_s_vec_2d pos, const e_item_type item_type) {
    return Collider(pos, ItemDropColliderSize(item_type), ITEM_DROP_ORIGIN);
}

#endif
