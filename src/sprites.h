#ifndef SPRITES_H
#define SPRITES_H

#include <zfw_rendering.h>
#include <zfw_math.h>
#include "assets.h"

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

    ek_sprite_dirt_particle,
    ek_sprite_stone_particle,
    ek_sprite_grass_particle,
    ek_sprite_gel_particle,

    ek_sprite_mouse,

    eks_sprite_cnt
} e_sprite;

typedef struct {
    e_texture tex;
    s_rect_i src_rect;
} s_sprite;

extern const s_sprite g_sprites[];

static inline void RenderSprite(const s_rendering_context* const context, const e_sprite spr, const s_textures* const textures, const s_vec_2d pos, const s_vec_2d origin, const s_vec_2d scale, const float rot, const s_color blend) {
    RenderTexture(context, g_sprites[spr].tex, textures, g_sprites[spr].src_rect, pos, origin, scale, rot, blend);
}

#endif
