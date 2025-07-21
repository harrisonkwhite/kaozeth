#ifndef LIGHTING_H
#define LIGHTING_H

#include <zfw_rendering.h>
#include <zfw_math.h>
#include <zfw_utils.h>

#define LIGHT_LEVEL_LIMIT 12

typedef int t_light_level;

typedef struct {
    t_light_level* buf;
    s_vec_2d_i size;
} s_lightmap;

s_lightmap GenLightmap(s_mem_arena* const mem_arena, const s_vec_2d_i size);
void PropagateLight(const s_lightmap* const lightmap, const s_vec_2d_i pos, const t_light_level level);
void RenderLightmap(const s_rendering_context* const rendering_context, const s_lightmap* const map, const s_vec_2d pos, const float tile_size);

#endif
