#include "game.h"
#include "zfw_utils.h"

const s_item g_items[] = {
    [ek_item_type_dirt_block] = {
        .name = "Dirt Block",
        .spr = ek_sprite_dirt_tile_item
    },
    [ek_item_type_stone_block] = {
        .name = "Stone Block",
        .spr = ek_sprite_stone_tile_item
    },
    [ek_item_type_wooden_sword] = {
        .name = "Wooden Sword",
        .spr = ek_sprite_dirt_tile
    }
};

static_assert(STATIC_ARRAY_LEN(g_items) == eks_item_type_cnt, "Invalid array length!");
