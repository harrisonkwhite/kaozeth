#include "tilemap.h"

#define TILEMAP_CONTACT_PRECISE_JUMP_SIZE 0.1f

void AddTile(s_tilemap_core* const tm_core, const zfw_s_vec_2d_s32 pos, const e_tile_type tile_type) {
    assert(IsTilePosInBounds(pos));
    assert(!IsTileActive(&tm_core->activity, pos));

    ActivateBit(IndexFrom2D(pos.x, pos.y, TILEMAP_WIDTH), (t_u8*)tm_core->activity, TILEMAP_WIDTH * TILEMAP_HEIGHT);
    tm_core->tile_types[pos.y][pos.x] = tile_type;
}

void RemoveTile(s_tilemap_core* const tm_core, const zfw_s_vec_2d_s32 pos) {
    assert(IsTilePosInBounds(pos));
    assert(IsTileActive(&tm_core->activity, pos));

    DeactivateBit(IndexFrom2D(pos.x, pos.y, TILEMAP_WIDTH), (t_u8*)tm_core->activity, TILEMAP_WIDTH * TILEMAP_HEIGHT);
}

zfw_s_rect_edges_s32 RectTilemapSpan(const zfw_s_rect rect) {
    assert(rect.width >= 0.0f && rect.height >= 0.0f);

    return ZFW_RectEdgesS32Clamped(
        (zfw_s_rect_edges_s32){
            rect.x / TILE_SIZE,
            rect.y / TILE_SIZE,
            ceilf((rect.x + rect.width) / TILE_SIZE),
            ceilf((rect.y + rect.height) / TILE_SIZE)
        },
        (zfw_s_rect_edges_s32){0, 0, TILEMAP_WIDTH, TILEMAP_HEIGHT}
    );
}

bool TileCollisionCheck(const t_tilemap_activity* const tm_activity, const zfw_s_rect collider) {
    assert(tm_activity);
    assert(collider.width > 0.0f && collider.height > 0.0f);

    const zfw_s_rect_edges_s32 collider_tilemap_span = RectTilemapSpan(collider);

    for (int ty = collider_tilemap_span.top; ty < collider_tilemap_span.bottom; ty++) {
        for (int tx = collider_tilemap_span.left; tx < collider_tilemap_span.right; tx++) {
            if (!IsTileActive(tm_activity, (zfw_s_vec_2d_s32){tx, ty})) {
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

void ProcTileCollisions(zfw_s_vec_2d* const pos, zfw_s_vec_2d* const vel, const zfw_s_vec_2d collider_size, const zfw_s_vec_2d collider_origin, const t_tilemap_activity* const tm_activity) {
    assert(pos);
    assert(vel);
    assert(collider_size.x > 0.0f && collider_size.y > 0.0f);
    assert(tm_activity);

    const zfw_s_rect hor_collider = Collider((zfw_s_vec_2d){pos->x + vel->x, pos->y}, collider_size, collider_origin);

    if (TileCollisionCheck(tm_activity, hor_collider)) {
        MakeContactWithTilemapByJumpSize(pos, TILEMAP_CONTACT_PRECISE_JUMP_SIZE, vel->x >= 0.0f ? zfw_ek_cardinal_dir_right : zfw_ek_cardinal_dir_left, collider_size, collider_origin, tm_activity);
        vel->x = 0.0f;
    }

    ProcVerTileCollisions(pos, &vel->y, collider_size, collider_origin, tm_activity);

    const zfw_s_rect diag_collider = Collider(ZFW_Vec2DSum(*pos, *vel), collider_size, collider_origin);

    if (TileCollisionCheck(tm_activity, diag_collider)) {
        vel->x = 0.0f;
    }
}

void ProcVerTileCollisions(zfw_s_vec_2d* const pos, float* const vel_y, const zfw_s_vec_2d collider_size, const zfw_s_vec_2d collider_origin, const t_tilemap_activity* const tm_activity) {
    assert(pos);
    assert(vel_y);
    assert(collider_size.x > 0.0f && collider_size.y > 0.0f);
    assert(tm_activity);

    const zfw_s_rect ver_collider = Collider((zfw_s_vec_2d){pos->x, pos->y + *vel_y}, collider_size, collider_origin);

    if (TileCollisionCheck(tm_activity, ver_collider)) {
        MakeContactWithTilemapByJumpSize(pos, TILEMAP_CONTACT_PRECISE_JUMP_SIZE, *vel_y >= 0.0f ? zfw_ek_cardinal_dir_down : zfw_ek_cardinal_dir_up, collider_size, collider_origin, tm_activity);
        *vel_y = 0.0f;
    }
}

void MakeContactWithTilemap(zfw_s_vec_2d* const pos, const zfw_e_cardinal_dir dir, const zfw_s_vec_2d collider_size, const zfw_s_vec_2d collider_origin, const t_tilemap_activity* const tm_activity) {
    // Jump by tile intervals first, then make more precise contact.
    MakeContactWithTilemapByJumpSize(pos, TILE_SIZE, dir, collider_size, collider_origin, tm_activity);
    MakeContactWithTilemapByJumpSize(pos, TILEMAP_CONTACT_PRECISE_JUMP_SIZE, dir, collider_size, collider_origin, tm_activity);
}

void MakeContactWithTilemapByJumpSize(zfw_s_vec_2d* const pos, const float jump_size, const zfw_e_cardinal_dir dir, const zfw_s_vec_2d collider_size, const zfw_s_vec_2d collider_origin, const t_tilemap_activity* const tm_activity) {
    assert(pos);
    assert(jump_size > 0.0f);
    assert(collider_size.x > 0.0f && collider_size.y > 0.0f);
    assert(tm_activity);

    const zfw_s_vec_2d_s32 jump_dir = zfw_g_cardinal_dir_vecs[dir];
    const zfw_s_vec_2d jump = {jump_dir.x * jump_size, jump_dir.y * jump_size};

    while (!TileCollisionCheck(tm_activity, Collider(ZFW_Vec2DSum(*pos, jump), collider_size, collider_origin))) {
        *pos = ZFW_Vec2DSum(*pos, jump);
    }
}

void RenderTilemap(const zfw_s_rendering_context* const rendering_context, const s_tilemap_core* const tilemap_core, const t_tilemap_tile_lifes* const tilemap_tile_lifes, const zfw_s_rect_edges_s32 range, const zfw_s_textures* const textures) {
    assert(IsTilemapRangeValid(range));

    for (int ty = range.top; ty < range.bottom; ty++) {
        for (int tx = range.left; tx < range.right; tx++) {
            if (!IsTileActive(&tilemap_core->activity, (zfw_s_vec_2d_s32){tx, ty})) {
                continue;
            }

            const s_tile_type* const tile_type = &g_tile_types[tilemap_core->tile_types[ty][tx]];
            const zfw_s_vec_2d tile_world_pos = {tx * TILE_SIZE, ty * TILE_SIZE};
            RenderSprite(rendering_context, tile_type->spr, textures, tile_world_pos, (zfw_s_vec_2d){0}, (zfw_s_vec_2d){1.0f, 1.0f}, 0.0f, ZFW_WHITE);

            // Render the break overlay.
            const int tile_life = (*tilemap_tile_lifes)[ty][tx];

            if (tile_life > 0) {
                const int tile_life_max = tile_type->life;
                const int break_spr_cnt = 4; // TODO: This is really bad. We need an animation frame system of some kind.
                const float break_index_mult = (float)tile_life / tile_life_max;
                const int break_index = break_spr_cnt * break_index_mult;
                assert(tile_life < tile_life_max); // Sanity check.

                RenderSprite(rendering_context, ek_sprite_tile_break_0 + break_index, textures, tile_world_pos, (zfw_s_vec_2d){0}, (zfw_s_vec_2d){1.0f, 1.0f}, 0.0f, ZFW_WHITE);
            }
        }
    }
}
