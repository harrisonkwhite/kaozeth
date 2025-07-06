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
        .use_type = ek_item_use_type_tile_destroy,
        .use_break = 10,
        .tile_destroy_range = 4
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

bool ProcItemUsage(s_world* const world, const s_input_state* const input_state, const s_vec_2d_i display_size) {
    assert(display_size.x > 0 && display_size.y > 0);

    if (world->player.item_use_break > 0) {
        world->player.item_use_break--;
        return true;
    }

    if (!IsMouseButtonDown(ek_mouse_button_code_left, input_state)) {
        return true;
    }

    s_inventory_slot* const cur_slot = &world->player_inv_slots[world->player_inv_hotbar_slot_selected];

    if (cur_slot->quantity == 0) {
        // Selected slot is empty, no item to use.
        return true;
    }

    const s_item_type* const active_item = &g_item_types[cur_slot->item_type];

    const s_vec_2d mouse_cam_pos = DisplayToCameraPos(input_state->mouse_pos, world->cam_pos, display_size);
    const s_vec_2d_i mouse_tile_pos = CameraToTilePos(mouse_cam_pos);

    bool used = false; // Did we use the item?

    switch (active_item->use_type) {
        case ek_item_use_type_tile_place:
            if (IsTilePosInBounds(mouse_tile_pos) && !IsTileActive(&world->core.tilemap_core.activity, mouse_tile_pos)) {
                PlaceTile(&world->core.tilemap_core, mouse_tile_pos, active_item->tile_place_type);
                used = true;
            }

            break;

        case ek_item_use_type_tile_destroy:
            {
                if (!IsTilePosInBounds(mouse_tile_pos) || !IsTileActive(&world->core.tilemap_core.activity, mouse_tile_pos)) {
                    break;
                }

                const s_vec_2d_i player_tile_pos = {
                    floorf(world->player.pos.x / TILE_SIZE),
                    floorf(world->player.pos.y / TILE_SIZE)
                };

                const s_vec_2d_i dist = {
                    ABS(mouse_tile_pos.x - player_tile_pos.x),
                    ABS(mouse_tile_pos.y - player_tile_pos.y)
                };

                if (dist.x > active_item->tile_destroy_range || dist.y > active_item->tile_destroy_range) {
                    break;
                }

                HurtTile(world, mouse_tile_pos);
                used = true;
            }

            break;

        case ek_item_use_type_shoot:
            {
                const s_vec_2d dir = Vec2DDir(world->player.pos, mouse_cam_pos);
                const s_vec_2d vel = Vec2DScaled(dir, active_item->shoot_proj_spd);

                if (!SpawnProjectile(world, active_item->shoot_proj_type, true, active_item->shoot_proj_dmg, world->player.pos, vel)) {
                    return false;
                }

                used = true;
            }

            break;
   }

    if (used) {
        if (active_item->consume_on_use) {
            cur_slot->quantity--;
        }

        world->player.item_use_break = active_item->use_break;
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
