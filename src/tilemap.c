#include "game.h"

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

void ProcTileCollisions(s_vec_2d* const vel, const s_rect collider, const t_tilemap_activity* const tm_activity) {
    assert(vel);
    assert(collider.width > 0 && collider.height > 0);
    assert(tm_activity);

    const s_rect hor_rect = RectTranslated(collider, (s_vec_2d){vel->x, 0.0f});

    if (TileCollisionCheck(tm_activity, hor_rect)) {
        vel->x = 0.0f;
    }

    const s_rect ver_rect = RectTranslated(collider, (s_vec_2d){0.0f, vel->y});

    if (TileCollisionCheck(tm_activity, ver_rect)) {
        vel->y = 0.0f;
    }

    const s_rect diag_rect = RectTranslated(collider, *vel);

    if (TileCollisionCheck(tm_activity, diag_rect)) {
        vel->x = 0.0f;
    }
}

void RenderTilemap(const s_rendering_context* const rendering_context, const t_tilemap_activity* const tm_activity, const s_rect_edges_i range, const s_textures* const textures) {
    assert(range.left >= 0 && range.left < TILEMAP_WIDTH);
    assert(range.right >= 0 && range.right <= TILEMAP_WIDTH);
    assert(range.top >= 0 && range.top < TILEMAP_HEIGHT);
    assert(range.bottom >= 0 && range.bottom <= TILEMAP_HEIGHT);
    assert(range.left <= range.right);
    assert(range.top <= range.bottom);

    for (int ty = range.top; ty < range.bottom; ty++) {
        for (int tx = range.left; tx < range.right; tx++) {
            if (!IsTileActive(tm_activity, (s_vec_2d_i){tx, ty})) {
                continue;
            }

            const s_vec_2d tile_world_pos = {tx * TILE_SIZE, ty * TILE_SIZE};
            RenderSprite(rendering_context, ek_sprite_dirt_tile, textures, tile_world_pos, VEC_2D_ZERO, (s_vec_2d){1.0f, 1.0f}, 0.0f, WHITE);
        }
    }
}
