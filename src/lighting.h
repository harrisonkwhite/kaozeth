#ifndef LIGHTING_H
#define LIGHTING_H

#include <zfw_rendering.h>
#include <zfw_math.h>
#include <zfw_utils.h>

#define LIGHT_LEVEL_LIMIT 16

typedef int t_light_level;

typedef struct {
    t_light_level* buf;
    s_vec_2d_i size;
} s_lightmap;

s_lightmap GenLightmap(s_mem_arena* const mem_arena, const s_vec_2d_i size, const t_byte* const seed, s_mem_arena* const temp_mem_arena);
void RenderLightmap(const s_rendering_context* const rendering_context, const s_lightmap* const map, const s_rect_edges_i range, const float tile_size);

#endif
