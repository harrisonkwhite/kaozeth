#ifndef GAME_H
#define GAME_H

#include <zfw.h>
#include "sprites.h"
#include "particles.h"

#define GAME_TITLE "Terraria"

#define BG_COLOR (u_v4){0.25f, 0.42f, 0.61f, 1.0f}

#define DEATH_TEXT "You were slain..."

#define TILE_SIZE 8
#define TILEMAP_WIDTH 640
#define TILEMAP_HEIGHT 128
#define WORLD_WIDTH (TILE_SIZE * TILEMAP_WIDTH)
#define WORLD_HEIGHT (TILE_SIZE * TILEMAP_HEIGHT)

#define GRAVITY 0.2f
#define CAMERA_LERP 0.3f

#define TILE_PLACE_DIST 4
#define TILE_HIGHLIGHT_ALPHA 0.4f

#define PLAYER_INIT_HP_MAX 100
#define PLAYER_ORIGIN (s_v2){0.5f, 0.5f}
#define PLAYER_HURT_FLASH_TIME 10

#define ITEM_DROP_ORIGIN (s_v2){0.5f, 0.5f}

#define POPUP_TEXT_LIMIT 1024
#define POPUP_TEXT_STR_BUF_SIZE 32
#define POPUP_TEXT_INACTIVITY_ALPHA_THRESH 0.001f
#define POPUP_TEXT_VEL_Y_MULT 0.9f
#define POPUP_TEXT_FADE_VEL_Y_ABS_THRESH 0.002f
#define POPUP_TEXT_ALPHA_MULT 0.9f

#define DMG_POPUP_TEXT_VEL_Y_MIN -4.0f
#define DMG_POPUP_TEXT_VEL_Y_MAX -2.5f
static_assert(DMG_POPUP_TEXT_VEL_Y_MIN <= DMG_POPUP_TEXT_VEL_Y_MAX, "Invalid range.");

#define MOUSE_HOVER_STR_BUF_SIZE 32
typedef char t_mouse_hover_str_buf[MOUSE_HOVER_STR_BUF_SIZE];

#define PLAYER_INVENTORY_COLUMN_CNT 6
#define PLAYER_INVENTORY_ROW_CNT 3
#define PLAYER_INVENTORY_LEN (PLAYER_INVENTORY_COLUMN_CNT * PLAYER_INVENTORY_ROW_CNT)
static_assert(PLAYER_INVENTORY_COLUMN_CNT <= 9, "Too large since each hotbar slot needs an associated digit key.");
#define TILE_PLACE_DEFAULT_USE_BREAK 2

#define NPC_LIMIT 256
#define NPC_ORIGIN (s_v2){0.5f, 0.5f}
#define NPC_HURT_FLASH_TIME 10

#define PROJECTILE_LIMIT 1024
#define ITEM_DROP_LIMIT 1024
#define WORLD_MEM_ARENA_SIZE ((1 << 20) * 2)

#define WORLD_NAME_LEN_LIMIT 20
#define WORLD_FILENAME_EXT ".wrld"
#define WORLD_FILENAME_BUF_SIZE (WORLD_NAME_LEN_LIMIT + sizeof(WORLD_FILENAME_EXT))
#define WORLD_LIMIT 3
typedef char t_world_filename[WORLD_FILENAME_BUF_SIZE];
typedef t_world_filename t_world_filenames[WORLD_LIMIT];
typedef char t_world_name_buf[WORLD_NAME_LEN_LIMIT + 1];

#define SETTINGS_FILENAME "settings.dat"
#define ITEM_QUANTITY_LIMIT 99 // TEMP: Eventually each item type will have its own unique quantity.

typedef enum {
    ek_surface_temp, // Used for anything short-lived, like player and NPC flash effects.
    eks_surface_cnt
} e_surface;

typedef enum {
    ek_tile_type_dirt,
    ek_tile_type_stone,
    ek_tile_type_grass,
    eks_tile_type_cnt
} e_tile_type;

typedef enum {
    ek_item_type_dirt_block,
    ek_item_type_stone_block,
    ek_item_type_grass_block,
    ek_item_type_copper_pickaxe,
    ek_item_type_wooden_sword,
    ek_item_type_wooden_bow,
    eks_item_type_cnt
} e_item_type;

typedef struct {
    e_item_type item_type;
    int quantity;
} s_inventory_slot;

typedef enum {
    ek_item_use_type_tile_place,
    ek_item_use_type_tile_hurt,
    ek_item_use_type_shoot
} e_item_use_type;

typedef enum {
    ek_projectile_type_wooden_arrow,
    eks_projectile_type_cnt
} e_projectile_type;

typedef enum {
    ek_projectile_type_flags_falls = 1 << 0,
    ek_projectile_type_flags_rot_is_dir = 1 << 1
} e_projectile_type_flags;

typedef enum {
    ek_npc_type_slime,
    eks_npc_type_cnt
} e_npc_type;

typedef enum {
    ek_title_screen_tick_result_type_normal,
    ek_title_screen_tick_result_type_error,
    ek_title_screen_tick_result_type_load_world,
    ek_title_screen_tick_result_type_exit
} e_title_screen_tick_result_type;

typedef enum {
    ek_title_screen_page_home,
    ek_title_screen_page_worlds,
    ek_title_screen_page_new_world,
    ek_title_screen_page_settings,
    eks_title_screen_page_cnt
} e_title_screen_page;

typedef enum {
    ek_setting_type_toggle,
    ek_setting_type_perc
} e_setting_type;

typedef enum {
    ek_setting_smooth_camera,
    ek_setting_volume,
    eks_setting_cnt
} e_setting;

typedef struct {
    e_sprite spr;
    e_item_type drop_item;
    int life;
    e_particle_template particle_template;
} s_tile_type;

extern const s_tile_type g_tile_types[];

typedef struct {
    const char* name;

    e_sprite icon_spr;

    bool consume_on_use;
    e_item_use_type use_type;
    int use_break;

    e_tile_type tile_place_type;

    int tile_hurt_dist;

    e_projectile_type shoot_proj_type;
    float shoot_proj_spd;
    int shoot_proj_dmg;
} s_item_type;

extern const s_item_type g_item_types[];

typedef struct {
    e_sprite spr;
    e_projectile_type_flags flags;
} s_projectile_type;

extern const s_projectile_type g_projectile_types[];

typedef struct {
    bool killed;
    s_v2 pos;
    s_v2 vel;
    bool jumping;
    int hp;
    int invinc_time;
    int item_use_break;
    int flash_time;
} s_player;

typedef t_byte t_npc_activity[BITS_TO_BYTES(NPC_LIMIT)];

typedef struct {
    int jump_time;
    float jump_hor_spd;
} s_slime_npc;

typedef union {
    s_slime_npc slime;
} u_npc_type_data;

typedef struct {
    s_v2 pos;
    s_v2 vel;
    int hp;
    int flash_time;
    e_npc_type type;
    u_npc_type_data type_data;
} s_npc;

typedef struct {
    s_npc buf[NPC_LIMIT];
    t_npc_activity activity;
} s_npcs;

typedef struct {
    e_projectile_type type;
    bool friendly;
    int dmg;
    s_v2 pos;
    s_v2 vel;
    float rot;
} s_projectile;

typedef struct {
    e_item_type item_type;
    int quantity;
    s_v2 pos;
    s_v2 vel;
} s_item_drop;

typedef struct {
    char str[POPUP_TEXT_STR_BUF_SIZE];
    s_v2 pos;
    float vel_y;
    float alpha;
} s_popup_text;

typedef struct {
    s_v2 pos;
    float scale;
} s_camera;

typedef t_byte t_tilemap_activity[BITS_TO_BYTES(TILEMAP_HEIGHT)][BITS_TO_BYTES(TILEMAP_WIDTH)];

typedef struct {
    t_tilemap_activity activity;
    e_tile_type tile_types[TILEMAP_HEIGHT][TILEMAP_WIDTH];
} s_tilemap_core;

typedef int t_tilemap_tile_lifes[TILEMAP_HEIGHT][TILEMAP_WIDTH];

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

    s_particles particles;

    s_camera cam;

    bool player_inv_open;
    s_inventory_slot player_inv_slots[PLAYER_INVENTORY_ROW_CNT][PLAYER_INVENTORY_COLUMN_CNT];
    int player_inv_hotbar_slot_selected;

    s_popup_text popup_texts[POPUP_TEXT_LIMIT];

    t_mouse_hover_str_buf mouse_hover_str;
    e_item_type mouse_item_held_type;
    int mouse_item_held_quantity;
} s_world;

typedef void (*t_npc_tick_func)(s_world* const world, const int npc_index);
typedef void (*t_npc_postspawn_func)(s_world* const world, const int npc_index);

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
    e_setting_type type;
    const char* name;
    t_byte preset;
} s_setting;

typedef t_byte t_settings[eks_setting_cnt];
extern const s_setting g_settings[];

typedef struct {
    e_title_screen_tick_result_type type;
    t_world_filename world_filename;
} s_title_screen_tick_result;

typedef struct {
    e_title_screen_page page;
    int page_btn_elem_hovered_index;
    t_world_filenames world_filenames_cache;
    t_world_name_buf new_world_name_buf;
} s_title_screen;

typedef struct {
    zfw_s_texture_group textures;
    zfw_s_font_group fonts;
    zfw_s_shader_prog_group shader_progs;
    zfw_s_sound_types snd_types;
    zfw_s_surface_group surfs;

    t_settings settings;

    bool in_world;
    s_title_screen title_screen;
    s_world world;
} s_game;

static inline float UIScale(const s_v2_int window_size) {
    if (window_size.x > 1920 && window_size.y > 1080) {
        return 2.0f;
    }

    if (window_size.x > 1600 && window_size.y > 900) {
        return 1.5f;
    }

    return 1.0f;
}

static inline s_v2_int UISize(const s_v2_int window_size) {
    assert(window_size.x > 0 && window_size.y > 0);
    const float ui_scale = UIScale(window_size);
    return (s_v2_int){window_size.x / ui_scale, window_size.y / ui_scale};
}

static inline s_v2 DisplayToUIPos(const s_v2 pos, const s_v2_int window_size) {
    const float ui_scale = UIScale(window_size);
    return (s_v2){pos.x / ui_scale, pos.y / ui_scale};
}

static inline zfw_s_rect Collider(const s_v2 pos, const s_v2 size, const s_v2 origin) {
    assert(size.x > 0.0f && size.y > 0.0f);
    return (zfw_s_rect){pos.x - (size.x * origin.x), pos.y - (size.y * origin.y), size.x, size.y};
}

static inline zfw_s_rect ColliderFromSprite(const e_sprite spr, const s_v2 pos, const s_v2 origin) {
    return Collider(pos, (s_v2){g_sprites[spr].src_rect.width, g_sprites[spr].src_rect.height}, origin);
}

static inline bool SettingToggle(const t_settings* const settings, const e_setting setting) {
    assert(g_settings[setting].type == ek_setting_type_toggle);
    return (*settings)[setting];
}

static inline float SettingPerc(const t_settings* const settings, const e_setting setting) {
    assert(g_settings[setting].type == ek_setting_type_perc);
    return (float)(*settings)[setting] / 100.0f;
}

static inline s_v2_int CameraToTilePos(const s_v2 pos) {
    return (s_v2_int){
        floorf(pos.x / TILE_SIZE),
        floorf(pos.y / TILE_SIZE)
    };
}

static inline s_v2 CameraSize(const float cam_scale, const s_v2_int window_size) {
    assert(cam_scale > 0.0f);
    assert(window_size.x > 0 && window_size.y > 0);
    return (s_v2){window_size.x / cam_scale, window_size.y / cam_scale};
}

static inline s_v2 CameraTopLeft(const s_camera* const cam, const s_v2_int window_size) {
    assert(window_size.x > 0 && window_size.y > 0);

    const s_v2 size = CameraSize(cam->scale, window_size);
    return (s_v2){cam->pos.x - (size.x / 2.0f), cam->pos.y - (size.y / 2.0f)};
}

static inline s_v2 CameraToDisplayPos(const s_v2 pos, const s_camera* const cam, const s_v2_int window_size) {
    assert(window_size.x > 0 && window_size.y > 0);

    const s_v2 cam_tl = CameraTopLeft(cam, window_size);
    return (s_v2) {
        (pos.x - cam_tl.x) * cam->scale,
        (pos.y - cam_tl.y) * cam->scale
    };
}

static inline s_v2 DisplayToCameraPos(const s_v2 pos, const s_camera* const cam, const s_v2_int window_size) {
    assert(window_size.x > 0 && window_size.y > 0);

    const s_v2 cam_tl = CameraTopLeft(cam, window_size);
    return (s_v2) {
        cam_tl.x + (pos.x / cam->scale),
        cam_tl.y + (pos.y / cam->scale)
    };
}

static inline s_v2 CameraToUIPos(const s_v2 pos, const s_camera* const cam, const s_v2_int window_size) {
    assert(window_size.x > 0 && window_size.y > 0);
    return DisplayToUIPos(CameraToDisplayPos(pos, cam, window_size), window_size);
}

//
// game.c
//
bool InitGame(const zfw_s_game_init_context* const zfw_context);
zfw_e_game_tick_result GameTick(const zfw_s_game_tick_context* const zfw_context);
bool RenderGame(const zfw_s_game_render_context* const zfw_context);
void CleanGame(void* const dev_mem);

//
// title_screen.c
//
bool InitTitleScreen(s_title_screen* const ts, s_mem_arena* const temp_mem_arena);
s_title_screen_tick_result TitleScreenTick(s_title_screen* const ts, t_settings* const settings, const zfw_s_game_tick_context* const zfw_context, const zfw_s_font_group* const fonts, const zfw_s_sound_types* const snd_types);
bool RenderTitleScreen(const s_title_screen* const ts, const zfw_s_rendering_context* const rendering_context, const t_settings* const settings, const zfw_s_texture_group* const textures, const zfw_s_font_group* const fonts, s_mem_arena* const temp_mem_arena);

//
// world.c
//
bool InitWorld(s_world* const world, const t_world_filename* const filename, const s_v2_int window_size, s_mem_arena* const temp_mem_arena);
void CleanWorld(s_world* const world);
bool WorldTick(s_world* const world, const t_settings* const settings, const zfw_s_game_tick_context* const zfw_context, const zfw_s_sound_types* const snd_types);
bool RenderWorld(const s_world* const world, const zfw_s_rendering_context* const rendering_context, const zfw_s_texture_group* const textures, const zfw_s_shader_prog_group* const shader_progs, zfw_s_surface_group* const surfs, s_mem_arena* const temp_mem_arena);
bool LoadWorldCoreFromFile(s_world_core* const world_core, const t_world_filename* const filename);
bool WriteWorldCoreToFile(const s_world_core* const world_core, const t_world_filename* const filename);
bool PlaceWorldTile(s_world* const world, const s_v2_int pos, const e_tile_type type);
bool HurtWorldTile(s_world* const world, const s_v2_int pos);
bool DestroyWorldTile(s_world* const world, const s_v2_int pos);
bool IsTilePosFree(const s_world* const world, const s_v2_int tile_pos);
s_popup_text* SpawnPopupText(s_world* const world, const s_v2 pos, const float vel_y);

//
// world_ui.c
//
void UpdateWorldUI(s_world* const world, const zfw_s_input_context* const input_context, const s_v2_int window_size);
bool RenderWorldUI(const s_world* const world, const zfw_s_game_render_context* const zfw_context, const zfw_s_texture_group* const textures, const zfw_s_font_group* const fonts);

//
// world_gen.c
//
void GenWorld(s_world_core* const world_core);

//
// player.c
//
void InitPlayer(s_player* const player, const int hp_max, const t_tilemap_activity* const tm_activity);
bool UpdatePlayer(s_world* const world, const zfw_s_input_context* const input_context);
void RenderPlayer(const s_player* const player, const zfw_s_rendering_context* const rendering_context, const zfw_s_texture_group* const textures, const zfw_s_shader_prog_group* const shader_progs, const zfw_s_surface_group* const surfs);
bool HurtPlayer(s_world* const world, const int dmg, const s_v2 kb);

static inline s_v2 PlayerColliderSize() {
    const s_sprite* const spr = &g_sprites[ek_sprite_player];
    return (s_v2){spr->src_rect.width, spr->src_rect.height};
}

static inline zfw_s_rect PlayerCollider(const s_v2 pos) {
    return Collider(pos, PlayerColliderSize(), PLAYER_ORIGIN);
}

//
// npcs.c
//
extern const s_npc_type g_npc_types[];

int SpawnNPC(s_world* const world, const s_v2 pos, const e_npc_type type, const t_tilemap_activity* const tm_activity);
void UpdateNPCs(s_world* const world);
void ProcNPCDeaths(s_world* const world);
void RenderNPCs(const s_npcs* const npcs, const zfw_s_rendering_context* const rendering_context, const zfw_s_texture_group* const textures, const zfw_s_shader_prog_group* const shader_progs, const zfw_s_surface_group* const surfs);
bool HurtNPC(s_world* const world, const int npc_index, const int dmg, const s_v2 kb);
bool IsNPCActive(const t_npc_activity* const activity, const int index);
bool ProcEnemySpawning(s_world* const world, const float cam_width);

static inline s_v2 NPCColliderSize(const e_npc_type npc_type) {
    const s_sprite* const spr = &g_sprites[g_npc_types[npc_type].spr];
    return (s_v2){spr->src_rect.width, spr->src_rect.height};
}

static inline zfw_s_rect NPCCollider(const s_v2 npc_pos, const e_npc_type npc_type) {
    return Collider(npc_pos, NPCColliderSize(npc_type), NPC_ORIGIN);
}

//
// projectiles.c
//
s_projectile* SpawnProjectile(s_world* const world, const e_projectile_type type, const bool friendly, const int dmg, const s_v2 pos, const s_v2 vel);
bool UpdateProjectiles(s_world* const world);
void RenderProjectiles(const s_projectile* const projectiles, const int proj_cnt, const zfw_s_rendering_context* const rendering_context, const zfw_s_texture_group* const textures);

static inline zfw_s_rect ProjectileCollider(const e_projectile_type proj_type, const s_v2 pos) {
    return ColliderFromSprite(g_projectile_types[proj_type].spr, pos, (s_v2){0.5f, 0.5f});
}

//
// items.c
//
bool IsItemUsable(const e_item_type item_type, const s_world* const world, const s_v2_int mouse_tile_pos);
bool ProcItemUsage(s_world* const world, const zfw_s_input_context* const input_context, const s_v2_int window_size);
bool SpawnItemDrop(s_world* const world, const s_v2 pos, const e_item_type item_type, const int item_quantity);
bool UpdateItemDrops(s_world* const world, zfw_s_audio_sys* const audio_sys, const zfw_s_sound_types* const snd_types, const t_settings* const settings);
void RenderItemDrops(const s_item_drop* const drops, const int drop_cnt, const zfw_s_rendering_context* const rendering_context, const zfw_s_texture_group* const textures);

static inline s_v2 ItemDropColliderSize(const e_item_type item_type) {
    const s_sprite* const spr = &g_sprites[g_item_types[item_type].icon_spr];
    return (s_v2){spr->src_rect.width, spr->src_rect.height};
}

static inline zfw_s_rect ItemDropCollider(const s_v2 pos, const e_item_type item_type) {
    return Collider(pos, ItemDropColliderSize(item_type), ITEM_DROP_ORIGIN);
}

//
// tilemap.c
//
void AddTile(s_tilemap_core* const tm_core, const s_v2_int pos, const e_tile_type tile_type);
void RemoveTile(s_tilemap_core* const tm_core, const s_v2_int pos);
zfw_s_rect_edges_int RectTilemapSpan(const zfw_s_rect rect);
bool TileCollisionCheck(const t_tilemap_activity* const tm_activity, const zfw_s_rect collider);
void ProcTileCollisions(s_v2* const pos, s_v2* const vel, const s_v2 collider_size, const s_v2 collider_origin, const t_tilemap_activity* const tm_activity);
void ProcVerTileCollisions(s_v2* const pos, float* const vel_y, const s_v2 collider_size, const s_v2 collider_origin, const t_tilemap_activity* const tm_activity);
void MakeContactWithTilemap(s_v2* const pos, const zfw_e_cardinal_dir dir, const s_v2 collider_size, const s_v2 collider_origin, const t_tilemap_activity* const tm_activity);
void MakeContactWithTilemapByJumpSize(s_v2* const pos, const float jump_size, const zfw_e_cardinal_dir dir, const s_v2 collider_size, const s_v2 collider_origin, const t_tilemap_activity* const tm_activity);
void RenderTilemap(const s_tilemap_core* const tilemap_core, const zfw_s_rendering_context* const rendering_context, const t_tilemap_tile_lifes* const tilemap_tile_lifes, const zfw_s_rect_edges_int range, const zfw_s_texture_group* const textures);

static inline bool IsTilePosInBounds(const s_v2_int pos) {
    return pos.x >= 0 && pos.x < TILEMAP_WIDTH && pos.y >= 0 && pos.y < TILEMAP_HEIGHT;
}

static inline bool IsTilemapRangeValid(const zfw_s_rect_edges_int range) {
    return ZFW_IsRangeIntValid(range, (s_v2_int){TILEMAP_WIDTH, TILEMAP_HEIGHT});
}

static inline int TileDist(const s_v2_int a, const s_v2_int b) {
    return Dist((s_v2){a.x, a.y}, (s_v2){b.x, b.y});
}

static bool IsTileActive(const t_tilemap_activity* const tm_activity, const s_v2_int pos) {
    assert(tm_activity);
    assert(IsTilePosInBounds(pos));
    return IsBitActive((const t_byte*)tm_activity, IndexFrom2D(pos.x, pos.y, TILEMAP_WIDTH), TILEMAP_WIDTH * TILEMAP_HEIGHT);
}

//
// inventory.c
//
int AddToInventory(s_inventory_slot* const slots, const int slot_cnt, const e_item_type item_type, int quantity);
int RemoveFromInventory(s_inventory_slot* const slots, const int slot_cnt, const e_item_type item_type, int quantity);
bool DoesInventoryHaveRoomFor(s_inventory_slot* const slots, const int slot_cnt, const e_item_type item_type, int quantity);

#endif
