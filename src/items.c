#include "game.h"
#include "zfw_utils.h"

const s_item_type g_item_types[] = {
    [ek_item_type_dirt_block] = {
        .name = "Dirt Block",
        .spr = ek_sprite_dirt_tile_item,
        .use_type = ek_item_use_type_tile_place,
        .consume_on_use = true,
        .tile_place_type = ek_tile_type_dirt
    },
    [ek_item_type_stone_block] = {
        .name = "Stone Block",
        .spr = ek_sprite_stone_tile_item,
        .use_type = ek_item_use_type_tile_place,
        .consume_on_use = true,
        .tile_place_type = ek_tile_type_stone
    },
    [ek_item_type_copper_pickaxe] = {
        .name = "Copper Pickaxe",
        .spr = ek_sprite_pickaxe_item,
        .use_type = ek_item_use_type_tile_destroy
    },
    [ek_item_type_wooden_sword] = {
        .name = "Wooden Sword",
        .spr = ek_sprite_dirt_tile,
        .use_type = ek_item_use_type_tile_place
    },
    [ek_item_type_wooden_bow] = {
        .name = "Wooden Bow",
        .spr = ek_sprite_dirt_tile,
        .use_type = ek_item_use_type_shoot,
        .shoot_proj_type = ek_projectile_type_wooden_arrow,
        .shoot_proj_spd = 7.0f,
        .shoot_proj_dmg = 3
    }
};

static_assert(STATIC_ARRAY_LEN(g_item_types) == eks_item_type_cnt, "Invalid array length!");
