#include "game.h"

#define TILEMAP_CONTACT_PRECISE_JUMP_SIZE 0.1f

const s_tile_type g_tile_types[] = {
    [ek_tile_type_dirt] = {
        .spr = ek_sprite_dirt_tile,
        .drop_item = ek_item_type_dirt_block,
        .life = 5
    },
    [ek_tile_type_stone] = {
        .spr = ek_sprite_stone_tile,
        .drop_item = ek_item_type_stone_block,
        .life = 8
    },
    [ek_tile_type_grass] = {
        .spr = ek_sprite_grass_tile,
        .drop_item = ek_item_type_grass_block,
        .life = 3
    }
};

static_assert(STATIC_ARRAY_LEN(g_tile_types) == eks_tile_type_cnt, "Invalid array length!");

static void ActivateTile(t_tilemap_activity* const tm_activity, const s_vec_2d_i pos) {
    assert(tm_activity);
    assert(IsTilePosInBounds(pos));
    ActivateBit(IndexFrom2D(pos, TILEMAP_WIDTH), (t_byte*)tm_activity, TILEMAP_WIDTH * TILEMAP_HEIGHT);
}

static void DeactivateTile(t_tilemap_activity* const tm_activity, const s_vec_2d_i pos) {
    assert(tm_activity);
    assert(IsTilePosInBounds(pos));
    DeactivateBit(IndexFrom2D(pos, TILEMAP_WIDTH), (t_byte*)tm_activity, TILEMAP_WIDTH * TILEMAP_HEIGHT);
}

s_rect_edges_i RectTilemapSpan(const s_rect rect) {
    assert(rect.width >= 0.0f && rect.height >= 0.0f);

    return RectEdgesIClamped(
        (s_rect_edges_i){
            rect.x / TILE_SIZE,
            rect.y / TILE_SIZE,
            ceilf((rect.x + rect.width) / TILE_SIZE),
            ceilf((rect.y + rect.height) / TILE_SIZE)
        },
        (s_rect_edges_i){0, 0, TILEMAP_WIDTH, TILEMAP_HEIGHT}
    );
}

void PlaceTile(s_tilemap_core* const tilemap, const s_vec_2d_i pos, const e_tile_type tile_type) {
    assert(tilemap);
    assert(IsTilePosInBounds(pos));
    assert(!IsTileActive(&tilemap->activity, pos));

    ActivateTile(&tilemap->activity, pos);
    tilemap->tile_types[pos.y][pos.x] = tile_type;
}

void HurtTile(s_world* const world, const s_vec_2d_i pos) {
    assert(world);
    assert(IsTilePosInBounds(pos));
    assert(IsTileActive(&world->core.tilemap_core.activity, pos));

    world->tilemap_tile_lifes[pos.y][pos.x]++;

    const s_tile_type* const tile_type = &g_tile_types[world->core.tilemap_core.tile_types[pos.y][pos.x]];

    if (world->tilemap_tile_lifes[pos.y][pos.x] == tile_type->life) {
        DestroyTile(world, pos);
    }
}

void DestroyTile(s_world* const world, const s_vec_2d_i pos) {
    assert(world);
    assert(IsTilePosInBounds(pos));
    assert(IsTileActive(&world->core.tilemap_core.activity, pos));

    DeactivateTile(&world->core.tilemap_core.activity, pos);

    const s_tile_type* const tile_type = &g_tile_types[world->core.tilemap_core.tile_types[pos.y][pos.x]];
    const s_vec_2d drop_pos = {(pos.x + 0.5f) * TILE_SIZE, (pos.y + 0.5f) * TILE_SIZE};
    SpawnItemDrop(world, drop_pos, tile_type->drop_item, 1);
}

bool IsTilePosFree(const s_world* const world, const s_vec_2d_i tile_pos) {
    assert(world);
    assert(IsTilePosInBounds(tile_pos));
    assert(!IsTileActive(&world->core.tilemap_core.activity, tile_pos));

    const s_rect tile_collider = {
        tile_pos.x * TILE_SIZE,
        tile_pos.y * TILE_SIZE,
        TILE_SIZE,
        TILE_SIZE
    };

    // Check for player.
    const s_rect player_collider = PlayerCollider(world->player.pos);

    if (DoRectsInters(tile_collider, player_collider)) {
        return false;
    }

    // Check for NPCs.
    for (int i = 0; i < NPC_LIMIT; i++) {
        if (!IsNPCActive(&world->npcs.activity, i)) {
            continue;
        }

        const s_npc* const npc = &world->npcs.buf[i];
        const s_rect npc_collider = NPCCollider(npc->pos, npc->type);

        if (DoRectsInters(tile_collider, npc_collider)) {
            return false;
        }
    }

    // Check for projectiles.
    for (int i = 0; i < world->proj_cnt; i++) {
        const s_projectile* const proj = &world->projectiles[i];
        const s_rect proj_collider = ProjectileCollider(proj->type, proj->pos);

        if (DoRectsInters(tile_collider, proj_collider)) {
            return false;
        }
    }

    return true;
}

bool TileCollisionCheck(const t_tilemap_activity* const tm_activity, const s_rect collider) {
    assert(tm_activity);
    assert(collider.width > 0.0f && collider.height > 0.0f);

    const s_rect_edges_i collider_tilemap_span = RectTilemapSpan(collider);

    for (int ty = collider_tilemap_span.top; ty < collider_tilemap_span.bottom; ty++) {
        for (int tx = collider_tilemap_span.left; tx < collider_tilemap_span.right; tx++) {
            if (!IsTileActive(tm_activity, (s_vec_2d_i){tx, ty})) {
                continue;
            }

            const s_rect tile_collider = {
                TILE_SIZE * tx,
                TILE_SIZE * ty,
                TILE_SIZE,
                TILE_SIZE
            };

            if (DoRectsInters(collider, tile_collider)) {
                return true;
            }
        }
    }

    return false;
}

void ProcTileCollisions(s_vec_2d* const pos, s_vec_2d* const vel, const s_vec_2d collider_size, const s_vec_2d collider_origin, const t_tilemap_activity* const tm_activity) {
    assert(pos);
    assert(vel);
    assert(collider_size.x > 0.0f && collider_size.y > 0.0f);
    assert(tm_activity);

    const s_rect hor_collider = Collider((s_vec_2d){pos->x + vel->x, pos->y}, collider_size, collider_origin);

    if (TileCollisionCheck(tm_activity, hor_collider)) {
        MakeContactWithTilemapByJumpSize(pos, TILEMAP_CONTACT_PRECISE_JUMP_SIZE, vel->x >= 0.0f ? ek_cardinal_dir_right : ek_cardinal_dir_left, collider_size, collider_origin, tm_activity);
        vel->x = 0.0f;
    }

    ProcVerTileCollisions(pos, &vel->y, collider_size, collider_origin, tm_activity);

    const s_rect diag_collider = Collider(Vec2DSum(*pos, *vel), collider_size, collider_origin);

    if (TileCollisionCheck(tm_activity, diag_collider)) {
        vel->x = 0.0f;
    }
}

void ProcVerTileCollisions(s_vec_2d* const pos, float* const vel_y, const s_vec_2d collider_size, const s_vec_2d collider_origin, const t_tilemap_activity* const tm_activity) {
    assert(pos);
    assert(vel_y);
    assert(collider_size.x > 0.0f && collider_size.y > 0.0f);
    assert(tm_activity);

    const s_rect ver_collider = Collider((s_vec_2d){pos->x, pos->y + *vel_y}, collider_size, collider_origin);

    if (TileCollisionCheck(tm_activity, ver_collider)) {
        MakeContactWithTilemapByJumpSize(pos, TILEMAP_CONTACT_PRECISE_JUMP_SIZE, *vel_y >= 0.0f ? ek_cardinal_dir_down : ek_cardinal_dir_up, collider_size, collider_origin, tm_activity);
        *vel_y = 0.0f;
    }
}

void MakeContactWithTilemap(s_vec_2d* const pos, const e_cardinal_dir dir, const s_vec_2d collider_size, const s_vec_2d collider_origin, const t_tilemap_activity* const tm_activity) {
    // Jump by tile intervals first, then make more precise contact.
    MakeContactWithTilemapByJumpSize(pos, TILE_SIZE, dir, collider_size, collider_origin, tm_activity);
    MakeContactWithTilemapByJumpSize(pos, TILEMAP_CONTACT_PRECISE_JUMP_SIZE, dir, collider_size, collider_origin, tm_activity);
}

void MakeContactWithTilemapByJumpSize(s_vec_2d* const pos, const float jump_size, const e_cardinal_dir dir, const s_vec_2d collider_size, const s_vec_2d collider_origin, const t_tilemap_activity* const tm_activity) {
    assert(pos);
    assert(jump_size > 0.0f);
    assert(collider_size.x > 0.0f && collider_size.y > 0.0f);
    assert(tm_activity);

    const s_vec_2d jump = Vec2DScaled(g_cardinal_dir_vecs[dir], jump_size);

    while (!TileCollisionCheck(tm_activity, Collider(Vec2DSum(*pos, jump), collider_size, collider_origin))) {
        *pos = Vec2DSum(*pos, jump);
    }
}

s_rect_edges_i TilemapRenderRange(const s_vec_2d cam_pos, const s_vec_2d_i display_size) {
    assert(display_size.x > 0 && display_size.y > 0);

    const s_vec_2d cam_tl = CameraTopLeft(cam_pos, display_size);
    const s_vec_2d cam_size = CameraSize(display_size);

    s_rect_edges_i render_range = {
        .left = floorf(cam_tl.x / TILE_SIZE),
        .top = floorf(cam_tl.y / TILE_SIZE),
        .right = ceilf((cam_tl.x + cam_size.x) / TILE_SIZE),
        .bottom = ceilf((cam_tl.y + cam_size.y) / TILE_SIZE)
    };

    // Clamp the tilemap render range within tilemap bounds.
    render_range.left = CLAMP(render_range.left, 0, TILEMAP_WIDTH - 1);
    render_range.top = CLAMP(render_range.top, 0, TILEMAP_HEIGHT - 1);
    render_range.right = CLAMP(render_range.right, 0, TILEMAP_WIDTH);
    render_range.bottom = CLAMP(render_range.bottom, 0, TILEMAP_HEIGHT);

    return render_range;
}

void RenderTilemap(const s_rendering_context* const rendering_context, const s_tilemap_core* const tilemap_core, const t_tilemap_tile_lifes* const tilemap_tile_lifes, const s_rect_edges_i range, const s_textures* const textures) {
    assert(IsTilemapRangeValid(range));

    for (int ty = range.top; ty < range.bottom; ty++) {
        for (int tx = range.left; tx < range.right; tx++) {
            if (!IsTileActive(&tilemap_core->activity, (s_vec_2d_i){tx, ty})) {
                continue;
            }

            const s_tile_type* const tile_type = &g_tile_types[tilemap_core->tile_types[ty][tx]];
            const s_vec_2d tile_world_pos = {tx * TILE_SIZE, ty * TILE_SIZE};
            RenderSprite(rendering_context, tile_type->spr, textures, tile_world_pos, VEC_2D_ZERO, (s_vec_2d){1.0f, 1.0f}, 0.0f, WHITE);

            // Render the break overlay.
            const int tile_life = (*tilemap_tile_lifes)[ty][tx];

            if (tile_life > 0) {
                const int tile_life_max = tile_type->life;
                const int break_spr_cnt = 4; // TODO: This is really bad. We need an animation frame system of some kind.
                const float break_index_mult = (float)tile_life / tile_life_max;
                const int break_index = break_spr_cnt * break_index_mult;
                assert(tile_life < tile_life_max); // Sanity check.

                RenderSprite(rendering_context, ek_sprite_tile_break_0 + break_index, textures, tile_world_pos, VEC_2D_ZERO, (s_vec_2d){1.0f, 1.0f}, 0.0f, WHITE);
            }
        }
    }
}

void RenderTileHighlight(const s_rendering_context* const rendering_context, const s_world* const world, const s_vec_2d mouse_pos) {
    const s_inventory_slot* const active_slot = &world->player_inv_slots[0][world->player_inv_hotbar_slot_selected];

    if (!world->player_inv_open && active_slot->quantity > 0) {
        const s_vec_2d mouse_cam_pos = DisplayToCameraPos(mouse_pos, world->cam_pos, rendering_context->display_size);
        const s_vec_2d_i mouse_tile_pos = CameraToTilePos(mouse_cam_pos);

        const s_item_type* const active_item = &g_item_types[active_slot->item_type];

        if ((active_item->use_type == ek_item_use_type_tile_place || active_item->use_type == ek_item_use_type_tile_hurt) && IsItemUsable(active_slot->item_type, world, mouse_tile_pos)) {
            const s_vec_2d mouse_cam_pos_snapped_to_tilemap = {mouse_tile_pos.x * TILE_SIZE, mouse_tile_pos.y * TILE_SIZE};

            const s_vec_2d highlight_pos = CameraToUIPos(mouse_cam_pos_snapped_to_tilemap, world->cam_pos, rendering_context->display_size);
            const float highlight_size = (float)(TILE_SIZE * CAMERA_SCALE) / UI_SCALE;
            const s_rect highlight_rect = {
                .x = highlight_pos.x,
                .y = highlight_pos.y,
                .width = highlight_size,
                .height = highlight_size
            };
            RenderRect(rendering_context, highlight_rect, (s_color){1.0f, 1.0f, 1.0f, TILE_HIGHLIGHT_ALPHA});
        }
    }
}
