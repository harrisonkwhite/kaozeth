#include <stdio.h>
#include "lighting.h"

static inline bool IsLightPosInBounds(const zfw_s_vec_2d_i pos, const zfw_s_vec_2d_i map_size) {
    return pos.x >= 0 && pos.x < map_size.x && pos.y >= 0 && pos.y < map_size.y;
}

static inline t_light_level LightLevel(const s_lightmap* const map, const zfw_s_vec_2d_i pos) {
    assert(IsLightPosInBounds(pos, map->size));
    return map->buf[ZFWIndexFrom2D(pos, map->size.x)];
}

s_lightmap GenLightmap(zfw_s_mem_arena* const mem_arena, const zfw_s_vec_2d_i size) {
    assert(size.x > 0 && size.y > 0);

    t_light_level* const buf = ZFW_MEM_ARENA_PUSH_TYPE_MANY(mem_arena, t_light_level, size.x * size.y);

    if (!buf) {
        fprintf(stderr, "Failed to allocate memory for lightmap!\n");
        return (s_lightmap){0};
    }

    return (s_lightmap){
        .buf = buf,
        .size = size
    };
}

void PropagateLight(const s_lightmap* const lightmap, const zfw_s_vec_2d_i pos, const t_light_level level) {
    assert(level > 0);

    if (!IsLightPosInBounds(pos, lightmap->size)) {
        return;
    }

    const int index = ZFWIndexFrom2D(pos, lightmap->size.x);

    if (lightmap->buf[index] >= level) {
        return;
    }

    lightmap->buf[index] = level;

    if (level > 1) {
        PropagateLight(lightmap, (zfw_s_vec_2d_i){pos.x + 1, pos.y}, level - 1);
        PropagateLight(lightmap, (zfw_s_vec_2d_i){pos.x - 1, pos.y}, level - 1);
        PropagateLight(lightmap, (zfw_s_vec_2d_i){pos.x, pos.y + 1}, level - 1);
        PropagateLight(lightmap, (zfw_s_vec_2d_i){pos.x, pos.y - 1}, level - 1);
    }
}

void RenderLightmap(const zfw_s_rendering_context* const rendering_context, const s_lightmap* const map, const zfw_s_vec_2d pos, const float tile_size) {
    assert(tile_size > 0.0f);

    for (int ty = 0; ty < map->size.y; ty++) {
        for (int tx = 0; tx < map->size.x; tx++) {
            const t_light_level light_lvl = LightLevel(map, (zfw_s_vec_2d_i){tx, ty});

            if (light_lvl == LIGHT_LEVEL_LIMIT) {
                continue;
            }

            const zfw_s_rect rect = {
                pos.x + (tx * tile_size),
                pos.y + (ty * tile_size),
                tile_size,
                tile_size
            };

            const zfw_s_color blend = {
                .a = 1.0f - ((float)light_lvl / LIGHT_LEVEL_LIMIT)
            };

            ZFWRenderRect(rendering_context, rect, blend);
        }
    }
}
