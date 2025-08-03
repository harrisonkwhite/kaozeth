#ifndef SPRITES_H
#define SPRITES_H

#include <zfw.h>
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
    zfw_s_rect_int src_rect;
} s_sprite;

static const s_sprite g_sprites[] = {
    [ek_sprite_player] = {
        .tex = ek_texture_player,
        .src_rect = {1, 1, 14, 22}
    },

    [ek_sprite_slime] = {
        .tex = ek_texture_npcs,
        .src_rect = {1, 1, 14, 14}
    },

    [ek_sprite_dirt_tile] = {
        .tex = ek_texture_tiles,
        .src_rect = {0, 0, 8, 8}
    },

    [ek_sprite_stone_tile] = {
        .tex = ek_texture_tiles,
        .src_rect = {8, 0, 8, 8}
    },

    [ek_sprite_grass_tile] = {
        .tex = ek_texture_tiles,
        .src_rect = {16, 0, 8, 8}
    },

    [ek_sprite_tile_break_0] = {
        .tex = ek_texture_tiles,
        .src_rect = {0, 8, 8, 8}
    },

    [ek_sprite_tile_break_1] = {
        .tex = ek_texture_tiles,
        .src_rect = {8, 8, 8, 8}
    },

    [ek_sprite_tile_break_2] = {
        .tex = ek_texture_tiles,
        .src_rect = {16, 8, 8, 8}
    },

    [ek_sprite_tile_break_3] = {
        .tex = ek_texture_tiles,
        .src_rect = {24, 8, 8, 8}
    },

    [ek_sprite_dirt_block_item_icon] = {
        .tex = ek_texture_item_icons,
        .src_rect = {1, 1, 6, 6}
    },

    [ek_sprite_stone_block_item_icon] = {
        .tex = ek_texture_item_icons,
        .src_rect = {9, 1, 6, 6}
    },

    [ek_sprite_grass_block_item_icon] = {
        .tex = ek_texture_item_icons,
        .src_rect = {17, 1, 6, 6}
    },

    [ek_sprite_copper_pickaxe_item_icon] = {
        .tex = ek_texture_item_icons,
        .src_rect = {2, 9, 12, 14}
    },

    [ek_sprite_item_icon_template] = {
        .tex = ek_texture_item_icons,
        .src_rect = {0, 24, 16, 16}
    },

    [ek_sprite_projectile] = {
        .tex = ek_texture_projectiles,
        .src_rect = {0, 2, 16, 4}
    },

    [ek_sprite_dirt_particle] = {
        .tex = ek_texture_particles,
        .src_rect = {2, 2, 4, 4}
    },

    [ek_sprite_stone_particle] = {
        .tex = ek_texture_particles,
        .src_rect = {10, 2, 4, 4}
    },

    [ek_sprite_grass_particle] = {
        .tex = ek_texture_particles,
        .src_rect = {18, 2, 4, 4}
    },

    [ek_sprite_gel_particle] = {
        .tex = ek_texture_particles,
        .src_rect = {26, 2, 4, 4}
    },

    [ek_sprite_mouse] = {
        .tex = ek_texture_misc,
        .src_rect = {2, 2, 4, 4}
    }
};

STATIC_ARRAY_LEN_CHECK(g_sprites, eks_sprite_cnt);

static inline void RenderSprite(const zfw_s_rendering_context* const context, const e_sprite spr, const zfw_s_texture_group* const textures, const zfw_s_vec_2d pos, const zfw_s_vec_2d origin, const zfw_s_vec_2d scale, const float rot, const zfw_u_vec_4d blend) {
    ZFW_RenderTexture(context, textures, g_sprites[spr].tex, g_sprites[spr].src_rect, pos, origin, scale, rot, blend);
}

#endif
