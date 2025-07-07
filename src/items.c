#include "game.h"
#include "zfw_rendering.h"

#define TILE_PLACE_DEFAULT_USE_BREAK 2

const s_item_type g_item_types[] = {
    [ek_item_type_dirt_block] = {
        .name = "Dirt Block",
        .icon_spr = ek_sprite_dirt_block_item_icon,
        .use_type = ek_item_use_type_tile_place,
        .use_break = TILE_PLACE_DEFAULT_USE_BREAK,
        .consume_on_use = true,
        .tile_place_type = ek_tile_type_dirt
    },

    [ek_item_type_stone_block] = {
        .name = "Stone Block",
        .icon_spr = ek_sprite_stone_block_item_icon,
        .use_type = ek_item_use_type_tile_place,
        .use_break = TILE_PLACE_DEFAULT_USE_BREAK,
        .consume_on_use = true,
        .tile_place_type = ek_tile_type_stone
    },

    [ek_item_type_sand_block] = {
        .name = "Sand Block",
        .icon_spr = ek_sprite_sand_block_item_icon,
        .use_type = ek_item_use_type_tile_place,
        .use_break = TILE_PLACE_DEFAULT_USE_BREAK,
        .consume_on_use = true,
        .tile_place_type = ek_tile_type_sand
    },

    [ek_item_type_copper_pickaxe] = {
        .name = "Copper Pickaxe",
        .icon_spr = ek_sprite_stone_block_item_icon,
        .use_type = ek_item_use_type_tile_hurt,
        .use_break = 10,
        .tile_hurt_dist = 4
    },

    [ek_item_type_wooden_sword] = {
        .name = "Wooden Sword",
        .icon_spr = ek_sprite_stone_block_item_icon,
        .use_type = ek_item_use_type_tile_place,
        .use_break = 10
    },

    [ek_item_type_wooden_bow] = {
        .name = "Wooden Bow",
        .icon_spr = ek_sprite_stone_block_item_icon,
        .use_type = ek_item_use_type_shoot,
        .use_break = 10,
        .shoot_proj_type = ek_projectile_type_wooden_arrow,
        .shoot_proj_spd = 7.0f,
        .shoot_proj_dmg = 3
    }
};

static_assert(STATIC_ARRAY_LEN(g_item_types) == eks_item_type_cnt, "Invalid array length!");

bool IsItemUsable(const e_item_type item_type, const s_vec_2d_i player_tile_pos, const s_vec_2d_i cursor_tile_pos, const t_tilemap_activity* const tm_activity) {
    const int player_to_cursor_tile_dist = TileDist(player_tile_pos, cursor_tile_pos);

    switch (g_item_types[item_type].use_type) {
        case ek_item_use_type_tile_place:
            if (!IsTilePosInBounds(cursor_tile_pos) || IsTileActive(tm_activity, cursor_tile_pos)) {
                return false;
            }

            return player_to_cursor_tile_dist <= TILE_PLACE_DIST;

        case ek_item_use_type_tile_hurt:
            if (!IsTilePosInBounds(cursor_tile_pos) || !IsTileActive(tm_activity, cursor_tile_pos)) {
                return false;
            }

            return player_to_cursor_tile_dist <= g_item_types[item_type].tile_hurt_dist;

        case ek_item_use_type_shoot:
            return true;
   }
}

bool ProcItemUsage(s_world* const world, const s_input_state* const input_state, const s_vec_2d_i display_size) {
    if (world->player.item_use_break > 0) {
        world->player.item_use_break--;
        return true;
    }

    s_inventory_slot* const slot = &world->player_inv_slots[world->player_inv_hotbar_slot_selected];

    if (slot->quantity == 0) {
        // Selected slot is empty, no item to use.
        return true;
    }

    const s_item_type* const item_type = &g_item_types[slot->item_type];

    const s_vec_2d_i player_tile_pos = CameraToTilePos(world->player.pos);

    const s_vec_2d cursor_cam_pos = DisplayToCameraPos(input_state->mouse_pos, world->cam_pos, display_size);
    const s_vec_2d_i cursor_tile_pos = CameraToTilePos(cursor_cam_pos);

    if (!IsItemUsable(slot->item_type, player_tile_pos, cursor_tile_pos, &world->core.tilemap_core.activity)) {
        return true;
    }

    if (!IsMouseButtonDown(ek_mouse_button_code_left, input_state)) {
        return true;
    }

    switch (item_type->use_type) {
        case ek_item_use_type_tile_place:
            PlaceTile(&world->core.tilemap_core, cursor_tile_pos, item_type->tile_place_type);
            break;

        case ek_item_use_type_tile_hurt:
            HurtTile(world, cursor_tile_pos);
            break;

        case ek_item_use_type_shoot:
            {
                const s_vec_2d dir = Vec2DDir(world->player.pos, cursor_cam_pos);
                const s_vec_2d vel = Vec2DScaled(dir, item_type->shoot_proj_spd);

                if (!SpawnProjectile(world, item_type->shoot_proj_type, true, item_type->shoot_proj_dmg, world->player.pos, vel)) {
                    return false;
                }
            }

            break;
    }

    world->player.item_use_break = item_type->use_break;

    if (item_type->consume_on_use) {
        slot->quantity--;
    }

    return true;
}

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

        ProcVerTileCollisions(&drop->pos, &drop->vel.y, ItemDropColliderSize(drop->item_type), ITEM_DROP_ORIGIN, &world->core.tilemap_core.activity);

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

void RenderItemDrops(const s_rendering_context* const rendering_context, const s_item_drop* const drops, const int drop_cnt, const s_textures* const textures) {
    assert(rendering_context);
    assert(drops);
    assert(drop_cnt >= 0);

    for (int i = 0; i < drop_cnt; i++) {
        const s_item_drop* const drop = &drops[i];
        const e_sprite spr = g_item_types[drop->item_type].icon_spr;
        RenderSprite(rendering_context, spr, textures, drop->pos, ITEM_DROP_ORIGIN, (s_vec_2d){1.0f, 1.0f}, 0.0f, WHITE);
    }
}
