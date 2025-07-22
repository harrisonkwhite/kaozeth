#ifndef GAME_H
#define GAME_H

#include <zfw_game.h>
#include <zfw_utils.h>
#include <zfw_rendering.h>
#include <zfw_random.h>
#include "particles.h"

#define GAME_TITLE "Terraria"

#define SETTINGS_FILENAME "settings.dat"

#define ITEM_QUANTITY_LIMIT 99 // TEMP

extern float g_ui_scale; // TEMP: Figure out a good system for readonly globals.

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
    e_setting_type type;
    const char* name;
    zfw_t_byte preset;
} s_setting;

typedef zfw_t_byte t_settings[eks_setting_cnt];

extern const s_setting g_settings[];

static inline bool SettingToggle(const t_settings* const settings, const e_setting setting) {
    assert(g_settings[setting].type == ek_setting_type_toggle);
    return (*settings)[setting];
}

static inline float SettingPerc(const t_settings* const settings, const e_setting setting) {
    assert(g_settings[setting].type == ek_setting_type_perc);
    return (float)(*settings)[setting] / 100.0f;
}

static inline zfw_s_vec_2d_i UISize(const zfw_s_vec_2d_i window_size) {
    assert(window_size.x > 0 && window_size.y > 0);
    return ZFWVec2DIScaled(window_size, 1.0f / g_ui_scale);
}

static inline zfw_s_vec_2d DisplayToUIPos(const zfw_s_vec_2d pos) {
    return ZFWVec2DScaled(pos, 1.0f / g_ui_scale);
}

static inline zfw_s_rect Collider(const zfw_s_vec_2d pos, const zfw_s_vec_2d size, const zfw_s_vec_2d origin) {
    assert(size.x > 0.0f && size.y > 0.0f);
    return (zfw_s_rect){pos.x - (size.x * origin.x), pos.y - (size.y * origin.y), size.x, size.y};
}

static inline zfw_s_rect ColliderFromSprite(const e_sprite spr, const zfw_s_vec_2d pos, const zfw_s_vec_2d origin) {
    return Collider(pos, (zfw_s_vec_2d){g_sprites[spr].src_rect.width, g_sprites[spr].src_rect.height}, origin);
}

//
//
//
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

extern const s_projectile_type g_projectile_types[];

//
//
//
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

//
//
//
typedef struct {
    e_sprite spr;
    e_item_type drop_item;
    int life;
    e_particle_template particle_template;
} s_tile_type;

//
//
//
typedef enum {
    ek_item_use_type_tile_place,
    ek_item_use_type_tile_hurt,
    ek_item_use_type_shoot
} e_item_use_type;

typedef struct {
    const char* name;
    const e_sprite icon_spr;

    bool consume_on_use;
    e_item_use_type use_type;
    int use_break;

    e_tile_type tile_place_type;

    int tile_hurt_dist;

    e_projectile_type shoot_proj_type;
    float shoot_proj_spd;
    int shoot_proj_dmg;
} s_item_type;

extern const s_tile_type g_tile_types[];
extern const s_item_type g_item_types[];

#endif
