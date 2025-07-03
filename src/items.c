#include "game.h"

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

bool SpawnItemDrop(s_world* const world, const s_vec_2d pos, const e_item_type item_type, const int item_quantity) {
    assert(world);
    assert(item_quantity > 0);

    if (world->item_drop_active_cnt == ITEM_DROP_LIMIT) {
        return false;
    }

    s_item_drop* const drop = &world->item_drops[world->item_drop_active_cnt];
    assert(IS_ZERO(*drop));
    drop->item_type = item_type;
    drop->quantity = item_quantity;
    drop->pos = pos;

    world->item_drop_active_cnt++;

    return true;
}

void UpdateItemDrops(s_world* const world) {
    assert(world);

    const s_rect player_collider = PlayerCollider(world->player.pos);

    for (int i = 0; i < world->item_drop_active_cnt; i++) {
        s_item_drop* const drop = &world->item_drops[i];

        // Process movement.
        drop->vel.y += GRAVITY;

        {
            const s_rect drop_collider = ItemDropCollider(drop->pos, drop->item_type);
            ProcVerTileCollisions(&drop->vel.y, drop_collider, &world->pers.tilemap.activity);
        }

        drop->pos = Vec2DSum(drop->pos, drop->vel);

        // Process collection.
        const bool collectable = DoesInventoryHaveRoomFor(world->player_inv_slots, PLAYER_INVENTORY_LENGTH, drop->item_type, drop->quantity);

        if (collectable) {
            const s_rect drop_collider = ItemDropCollider(drop->pos, drop->item_type);

            if (DoRectsInters(player_collider, drop_collider)) {
                AddToInventory(world->player_inv_slots, PLAYER_INVENTORY_LENGTH, drop->item_type, drop->quantity);

                // Remove this item drop.
                world->item_drop_active_cnt--;
                world->item_drops[i] = world->item_drops[world->item_drop_active_cnt];
                ZERO_OUT(world->item_drops[world->item_drop_active_cnt]);

                i--;
            }
        }
    }
}
