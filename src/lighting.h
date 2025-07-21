#ifndef LIGHTING_H
#define LIGHTING_H

#include <zfw_rendering.h>
#include <zfw_math.h>
#include <zfw_utils.h>

typedef char t_light_level;

#define LIGHT_LEVEL_LIMIT (t_light_level)12

typedef struct {
    t_light_level* buf;
    zfw_s_vec_2d_i size;
} s_lightmap;

s_lightmap GenLightmap(zfw_s_mem_arena* const mem_arena, const zfw_s_vec_2d_i size);
bool PropagateLights(const s_lightmap* const lightmap, zfw_s_mem_arena* const temp_mem_arena);
void RenderLightmap(const zfw_s_rendering_context* const rendering_context, const s_lightmap* const map, const zfw_s_vec_2d pos, const float tile_size);

static inline bool IsLightPosInBounds(const zfw_s_vec_2d_i pos, const zfw_s_vec_2d_i map_size) {
    return pos.x >= 0 && pos.x < map_size.x && pos.y >= 0 && pos.y < map_size.y;
}

static inline t_light_level LightLevel(const s_lightmap* const map, const zfw_s_vec_2d_i pos) {
    assert(IsLightPosInBounds(pos, map->size));
    return map->buf[ZFWIndexFrom2D(pos, map->size.x)];
}

static inline bool IsLightLevelValid(const t_light_level lvl) {
    return lvl >= 0 && lvl <= LIGHT_LEVEL_LIMIT;
}

static inline void SetLightLevel(const s_lightmap* const map, const zfw_s_vec_2d_i pos, const t_light_level lvl) {
    assert(IsLightPosInBounds(pos, map->size));
    assert(IsLightLevelValid(lvl));

    map->buf[ZFWIndexFrom2D(pos, map->size.x)] = lvl;
}

static inline bool IsLightmapInitted(const s_lightmap* const map) {
    return map->buf && map->size.x > 0 && map->size.y > 0;
}

#endif
