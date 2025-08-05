#include "game.h"

#define TILEMAP_CONTACT_PRECISE_JUMP_SIZE 0.1f

const s_tile_type g_tile_types[] = {
    [ek_tile_type_dirt] = {
        .spr = ek_sprite_dirt_tile,
        .drop_item = ek_item_type_dirt_block,
        .life = 5,
        .particle_template = ek_particle_template_dirt
    },
    [ek_tile_type_stone] = {
        .spr = ek_sprite_stone_tile,
        .drop_item = ek_item_type_stone_block,
        .life = 8,
        .particle_template = ek_particle_template_stone
    },
    [ek_tile_type_grass] = {
        .spr = ek_sprite_grass_tile,
        .drop_item = ek_item_type_grass_block,
        .life = 3,
        .particle_template = ek_particle_template_grass
    }
};

STATIC_ARRAY_LEN_CHECK(g_tile_types, eks_tile_type_cnt);

void AddTile(s_tilemap_core* const tm_core, const s_v2_int pos, const e_tile_type tile_type) {
    assert(IsTilePosInBounds(pos));
    assert(!IsTileActive(&tm_core->activity, pos));

    ActivateBit((t_byte*)tm_core->activity, IndexFrom2D(pos.x, pos.y, TILEMAP_WIDTH), TILEMAP_WIDTH * TILEMAP_HEIGHT);
    tm_core->tile_types[pos.y][pos.x] = tile_type;
}

void RemoveTile(s_tilemap_core* const tm_core, const s_v2_int pos) {
    assert(IsTilePosInBounds(pos));
    assert(IsTileActive(&tm_core->activity, pos));

    DeactivateBit((t_byte*)tm_core->activity, IndexFrom2D(pos.x, pos.y, TILEMAP_WIDTH), TILEMAP_WIDTH * TILEMAP_HEIGHT);
}

zfw_s_rect_edges_int RectTilemapSpan(const zfw_s_rect rect) {
    assert(rect.width >= 0.0f && rect.height >= 0.0f);

    return ZFW_RectEdgesIntClamped(
        (zfw_s_rect_edges_int){
            rect.x / TILE_SIZE,
            rect.y / TILE_SIZE,
            ceilf((rect.x + rect.width) / TILE_SIZE),
            ceilf((rect.y + rect.height) / TILE_SIZE)
        },
        (zfw_s_rect_edges_int){0, 0, TILEMAP_WIDTH, TILEMAP_HEIGHT}
    );
}

bool TileCollisionCheck(const t_tilemap_activity* const tm_activity, const zfw_s_rect collider) {
    assert(tm_activity);
    assert(collider.width > 0.0f && collider.height > 0.0f);

    const zfw_s_rect_edges_int collider_tilemap_span = RectTilemapSpan(collider);

    for (int ty = collider_tilemap_span.top; ty < collider_tilemap_span.bottom; ty++) {
        for (int tx = collider_tilemap_span.left; tx < collider_tilemap_span.right; tx++) {
            if (!IsTileActive(tm_activity, (s_v2_int){tx, ty})) {
                continue;
            }

            const zfw_s_rect tile_collider = {
                TILE_SIZE * tx,
                TILE_SIZE * ty,
                TILE_SIZE,
                TILE_SIZE
            };

            if (ZFW_DoRectsInters(collider, tile_collider)) {
                return true;
            }
        }
    }

    return false;
}

void ProcTileCollisions(s_v2* const pos, s_v2* const vel, const s_v2 collider_size, const s_v2 collider_origin, const t_tilemap_activity* const tm_activity) {
    assert(pos);
    assert(vel);
    assert(collider_size.x > 0.0f && collider_size.y > 0.0f);
    assert(tm_activity);

    const zfw_s_rect hor_collider = Collider((s_v2){pos->x + vel->x, pos->y}, collider_size, collider_origin);

    if (TileCollisionCheck(tm_activity, hor_collider)) {
        MakeContactWithTilemapByJumpSize(pos, TILEMAP_CONTACT_PRECISE_JUMP_SIZE, vel->x >= 0.0f ? zfw_ek_cardinal_dir_right : zfw_ek_cardinal_dir_left, collider_size, collider_origin, tm_activity);
        vel->x = 0.0f;
    }

    ProcVerTileCollisions(pos, &vel->y, collider_size, collider_origin, tm_activity);

    const zfw_s_rect diag_collider = Collider(V2Sum(*pos, *vel), collider_size, collider_origin);

    if (TileCollisionCheck(tm_activity, diag_collider)) {
        vel->x = 0.0f;
    }
}

void ProcVerTileCollisions(s_v2* const pos, float* const vel_y, const s_v2 collider_size, const s_v2 collider_origin, const t_tilemap_activity* const tm_activity) {
    assert(pos);
    assert(vel_y);
    assert(collider_size.x > 0.0f && collider_size.y > 0.0f);
    assert(tm_activity);

    const zfw_s_rect ver_collider = Collider((s_v2){pos->x, pos->y + *vel_y}, collider_size, collider_origin);

    if (TileCollisionCheck(tm_activity, ver_collider)) {
        MakeContactWithTilemapByJumpSize(pos, TILEMAP_CONTACT_PRECISE_JUMP_SIZE, *vel_y >= 0.0f ? zfw_ek_cardinal_dir_down : zfw_ek_cardinal_dir_up, collider_size, collider_origin, tm_activity);
        *vel_y = 0.0f;
    }
}

void MakeContactWithTilemap(s_v2* const pos, const zfw_e_cardinal_dir dir, const s_v2 collider_size, const s_v2 collider_origin, const t_tilemap_activity* const tm_activity) {
    // Jump by tile intervals first, then make more precise contact.
    MakeContactWithTilemapByJumpSize(pos, TILE_SIZE, dir, collider_size, collider_origin, tm_activity);
    MakeContactWithTilemapByJumpSize(pos, TILEMAP_CONTACT_PRECISE_JUMP_SIZE, dir, collider_size, collider_origin, tm_activity);
}

void MakeContactWithTilemapByJumpSize(s_v2* const pos, const float jump_size, const zfw_e_cardinal_dir dir, const s_v2 collider_size, const s_v2 collider_origin, const t_tilemap_activity* const tm_activity) {
    assert(pos);
    assert(jump_size > 0.0f);
    assert(collider_size.x > 0.0f && collider_size.y > 0.0f);
    assert(tm_activity);

    const s_v2_int jump_dir = zfw_g_cardinal_dirs[dir];
    const s_v2 jump = {jump_dir.x * jump_size, jump_dir.y * jump_size};

    while (!TileCollisionCheck(tm_activity, Collider(V2Sum(*pos, jump), collider_size, collider_origin))) {
        *pos = V2Sum(*pos, jump);
    }
}

void RenderTilemap(const s_tilemap_core* const tilemap_core, const zfw_s_rendering_context* const rendering_context, const t_tilemap_tile_lifes* const tilemap_tile_lifes, const zfw_s_rect_edges_int range, const zfw_s_texture_group* const textures) {
    assert(IsTilemapRangeValid(range));

    for (int ty = range.top; ty < range.bottom; ty++) {
        for (int tx = range.left; tx < range.right; tx++) {
            if (!IsTileActive(&tilemap_core->activity, (s_v2_int){tx, ty})) {
                continue;
            }

            const s_tile_type* const tile_type = &g_tile_types[tilemap_core->tile_types[ty][tx]];
            const s_v2 tile_world_pos = {tx * TILE_SIZE, ty * TILE_SIZE};
            RenderSprite(rendering_context, tile_type->spr, textures, tile_world_pos, (s_v2){0}, (s_v2){1.0f, 1.0f}, 0.0f, ZFW_WHITE);

            // Render the break overlay.
            const int tile_life = (*tilemap_tile_lifes)[ty][tx];

            if (tile_life > 0) {
                const int tile_life_max = tile_type->life;
                const int break_spr_cnt = 4; // TODO: This is really bad. We need an animation frame system of some kind.
                const float break_index_mult = (float)tile_life / tile_life_max;
                const int break_index = break_spr_cnt * break_index_mult;
                assert(tile_life < tile_life_max); // Sanity check.

                RenderSprite(rendering_context, ek_sprite_tile_break_0 + break_index, textures, tile_world_pos, (s_v2){0}, (s_v2){1.0f, 1.0f}, 0.0f, ZFW_WHITE);
            }
        }
    }
}
