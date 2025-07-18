#ifndef GAME_H
#define GAME_H

#include <zfw_game.h>
#include <zfw_utils.h>
#include <zfw_random.h>

#define GAME_TITLE "Terraria"

#define SETTINGS_FILENAME "settings.dat"

#define ITEM_QUANTITY_LIMIT 99 // TEMP

#define UI_SCALE 1.0f

typedef enum {
    ek_texture_player,
    ek_texture_npcs,
    ek_texture_tiles,
    ek_texture_item_icons,
    ek_texture_projectiles,
    ek_texture_misc,

    eks_texture_cnt
} e_texture;

typedef enum {
    ek_font_eb_garamond_20,
    ek_font_eb_garamond_24,
    ek_font_eb_garamond_28,
    ek_font_eb_garamond_32,
    ek_font_eb_garamond_48,
    ek_font_eb_garamond_80,

    eks_font_cnt
} e_font;

typedef enum {
    ek_sound_type_button_click,
    ek_sound_type_item_drop_collect,

    eks_sound_type_cnt
} e_sound_type;

typedef enum {
    ek_sprite_player,

    ek_sprite_slime,

    ek_sprite_dirt_tile,
    ek_sprite_stone_tile,
    ek_sprite_grass_tile,
    ek_sprite_tile_break_0,
    ek_sprite_tile_break_1,
    ek_sprite_tile_break_2,
    ek_sprite_tile_break_3,

    ek_sprite_dirt_block_item_icon,
    ek_sprite_stone_block_item_icon,
    ek_sprite_grass_block_item_icon,
    ek_sprite_copper_pickaxe_item_icon,
    ek_sprite_item_icon_template,

    ek_sprite_projectile,

    ek_sprite_mouse,

    eks_sprite_cnt
} e_sprite;

typedef struct {
    e_texture tex;
    s_rect_i src_rect;
} s_sprite;

extern const s_sprite g_sprites[];

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
    t_byte preset;
} s_setting;

typedef t_byte t_settings[eks_setting_cnt];

extern const s_setting g_settings[];

static inline bool SettingToggle(const t_settings* const settings, const e_setting setting) {
    assert(g_settings[setting].type == ek_setting_type_toggle);
    return (*settings)[setting];
}

static inline float SettingPerc(const t_settings* const settings, const e_setting setting) {
    assert(g_settings[setting].type == ek_setting_type_perc);
    return (float)(*settings)[setting] / 100.0f;
}

static inline void RenderSprite(const s_rendering_context* const context, const e_sprite spr, const s_textures* const textures, const s_vec_2d pos, const s_vec_2d origin, const s_vec_2d scale, const float rot, const s_color blend) {
    RenderTexture(context, g_sprites[spr].tex, textures, g_sprites[spr].src_rect, pos, origin, scale, rot, blend);
}

static inline s_vec_2d_i UISize(const s_vec_2d_i display_size) {
    assert(display_size.x > 0 && display_size.y > 0);
    return Vec2DIScaled(display_size, 1.0f / UI_SCALE);
}

static inline s_vec_2d DisplayToUIPos(const s_vec_2d pos) {
    return Vec2DScaled(pos, 1.0f / UI_SCALE);
}

static inline s_rect Collider(const s_vec_2d pos, const s_vec_2d size, const s_vec_2d origin) {
    assert(size.x > 0.0f && size.y > 0.0f);
    return (s_rect){pos.x - (size.x * origin.x), pos.y - (size.y * origin.y), size.x, size.y};
}

static inline s_rect ColliderFromSprite(const e_sprite spr, const s_vec_2d pos, const s_vec_2d origin) {
    return Collider(pos, (s_vec_2d){g_sprites[spr].src_rect.width, g_sprites[spr].src_rect.height}, origin);
}

//
//
//

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
