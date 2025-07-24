#ifndef TILEMAP_H
#define TILEMAP_H

#include <zfw.h>
#include "game.h"

#define TILEMAP_WIDTH 640 // TEMP: Make dynamic!
#define TILEMAP_HEIGHT 128 // TEMP: Make dynamic!

#define CHUNK_WIDTH 100

#define TILE_SIZE 8

typedef zfw_t_byte t_tilemap_activity[ZFW_BITS_TO_BYTES(TILEMAP_HEIGHT)][ZFW_BITS_TO_BYTES(TILEMAP_WIDTH)];

typedef int t_tilemap_tile_lifes[TILEMAP_HEIGHT][TILEMAP_WIDTH];

typedef struct {
    t_tilemap_activity activity;
    e_tile_type tile_types[TILEMAP_HEIGHT][TILEMAP_WIDTH];
} s_tilemap_core;

void AddTile(s_tilemap_core* const tm_core, const zfw_s_vec_2d_i pos, const e_tile_type tile_type);
void RemoveTile(s_tilemap_core* const tm_core, const zfw_s_vec_2d_i pos);
zfw_s_rect_edges_i RectTilemapSpan(const zfw_s_rect rect);
bool TileCollisionCheck(const t_tilemap_activity* const tm_activity, const zfw_s_rect collider);
void ProcTileCollisions(zfw_s_vec_2d* const pos, zfw_s_vec_2d* const vel, const zfw_s_vec_2d collider_size, const zfw_s_vec_2d collider_origin, const t_tilemap_activity* const tm_activity);
void ProcVerTileCollisions(zfw_s_vec_2d* const pos, float* const vel_y, const zfw_s_vec_2d collider_size, const zfw_s_vec_2d collider_origin, const t_tilemap_activity* const tm_activity);
void MakeContactWithTilemap(zfw_s_vec_2d* const pos, const zfw_e_cardinal_dir dir, const zfw_s_vec_2d collider_size, const zfw_s_vec_2d collider_origin, const t_tilemap_activity* const tm_activity);
void MakeContactWithTilemapByJumpSize(zfw_s_vec_2d* const pos, const float jump_size, const zfw_e_cardinal_dir dir, const zfw_s_vec_2d collider_size, const zfw_s_vec_2d collider_origin, const t_tilemap_activity* const tm_activity);
void RenderTilemap(const zfw_s_rendering_context* const rendering_context, const s_tilemap_core* const tilemap_core, const t_tilemap_tile_lifes* const tilemap_tile_lifes, const zfw_s_rect_edges_i range, const zfw_s_textures* const textures);

static inline bool IsTilePosInBounds(const zfw_s_vec_2d_i pos) {
    return pos.x >= 0 && pos.x < TILEMAP_WIDTH && pos.y >= 0 && pos.y < TILEMAP_HEIGHT;
}

static inline bool IsTilemapRangeValid(const zfw_s_rect_edges_i range) {
    return ZFW_IsRangeValid(range, (zfw_s_vec_2d_i){TILEMAP_WIDTH, TILEMAP_HEIGHT});
}

static inline int TileDist(const zfw_s_vec_2d_i a, const zfw_s_vec_2d_i b) {
    return ZFW_Dist((zfw_s_vec_2d){a.x, a.y}, (zfw_s_vec_2d){b.x, b.y});
}

static bool IsTileActive(const t_tilemap_activity* const tm_activity, const zfw_s_vec_2d_i pos) {
    assert(tm_activity);
    assert(IsTilePosInBounds(pos));
    return ZFW_IsBitActive(ZFW_IndexFrom2D(pos, TILEMAP_WIDTH), (zfw_t_byte*)tm_activity, TILEMAP_WIDTH * TILEMAP_HEIGHT);
}

#endif
