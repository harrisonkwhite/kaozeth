#ifndef TILEMAP_H
#define TILEMAP_H

#include "game.h"

#define TILEMAP_WIDTH 640 // TEMP: Make dynamic!
#define TILEMAP_HEIGHT 200 // TEMP: Make dynamic!

#define TILE_SIZE 8

typedef t_byte t_tilemap_activity[BITS_TO_BYTES(TILEMAP_HEIGHT)][BITS_TO_BYTES(TILEMAP_WIDTH)];

typedef int t_tilemap_tile_lifes[TILEMAP_HEIGHT][TILEMAP_WIDTH];

typedef struct {
    t_tilemap_activity activity;
    e_tile_type tile_types[TILEMAP_HEIGHT][TILEMAP_WIDTH];
} s_tilemap_core;

void AddTile(s_tilemap_core* const tilemap, const s_vec_2d_i pos, const e_tile_type tile_type);
void RemoveTile(s_tilemap_core* const tilemap, const s_vec_2d_i pos);
s_rect_edges_i RectTilemapSpan(const s_rect rect);
bool TileCollisionCheck(const t_tilemap_activity* const tm_activity, const s_rect collider);
void ProcTileCollisions(s_vec_2d* const pos, s_vec_2d* const vel, const s_vec_2d collider_size, const s_vec_2d collider_origin, const t_tilemap_activity* const tm_activity);
void ProcVerTileCollisions(s_vec_2d* const pos, float* const vel_y, const s_vec_2d collider_size, const s_vec_2d collider_origin, const t_tilemap_activity* const tm_activity);
void MakeContactWithTilemap(s_vec_2d* const pos, const e_cardinal_dir dir, const s_vec_2d collider_size, const s_vec_2d collider_origin, const t_tilemap_activity* const tm_activity);
void MakeContactWithTilemapByJumpSize(s_vec_2d* const pos, const float jump_size, const e_cardinal_dir dir, const s_vec_2d collider_size, const s_vec_2d collider_origin, const t_tilemap_activity* const tm_activity);
void RenderTilemap(const s_rendering_context* const rendering_context, const s_tilemap_core* const tilemap_core, const t_tilemap_tile_lifes* const tilemap_tile_lifes, const s_rect_edges_i range, const s_textures* const textures);

static inline bool IsTilePosInBounds(const s_vec_2d_i pos) {
    return pos.x >= 0 && pos.x < TILEMAP_WIDTH && pos.y >= 0 && pos.y < TILEMAP_HEIGHT;
}

static inline bool IsTilemapRangeValid(const s_rect_edges_i range) {
    return IsRangeValid(range, (s_vec_2d_i){TILEMAP_WIDTH, TILEMAP_HEIGHT});
}

static inline int TileDist(const s_vec_2d_i a, const s_vec_2d_i b) {
    return Dist((s_vec_2d){a.x, a.y}, (s_vec_2d){b.x, b.y});
}

static bool IsTileActive(const t_tilemap_activity* const tm_activity, const s_vec_2d_i pos) {
    assert(tm_activity);
    assert(IsTilePosInBounds(pos));
    return IsBitActive(IndexFrom2D(pos, TILEMAP_WIDTH), (t_byte*)tm_activity, TILEMAP_WIDTH * TILEMAP_HEIGHT);
}

#endif
