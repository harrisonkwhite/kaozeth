#include <stdio.h>
#include "lighting.h"
#include "zfw_math.h"

typedef struct {
    s_vec_2d_i* buf;
    int cap;
    int start;
    int len;
} s_light_pos_queue;

static inline bool IsLightLevelValid(const t_light_level lvl) {
    return lvl >= 0 && lvl <= LIGHT_LEVEL_LIMIT;
}

static inline bool IsLightPosInBounds(const s_vec_2d_i pos, const s_vec_2d_i map_size) {
    return pos.x >= 0 && pos.x < map_size.x && pos.y >= 0 && pos.y < map_size.y;
}

static bool IsLightPosQueueValid(const s_light_pos_queue* const queue, const s_vec_2d_i map_size) {
    if (!queue->buf
        || queue->cap <= 0
        || queue->start < 0 || queue->start >= queue->cap
        || queue->len < 0 || queue->len > queue->cap) {
        return false;
    }

    for (int i = 0; i < queue->len; i++) {
        const int bi = (queue->start + i) % queue->cap;
        const s_vec_2d_i light = queue->buf[bi];

        if (!IsLightPosInBounds(light, map_size)) {
            return false;
        }
    }

    return true;
}

static bool EnqueueLightPos(s_light_pos_queue* const queue, const s_vec_2d_i light_pos, const s_vec_2d_i map_size) {
    assert(IsLightPosQueueValid(queue, map_size));
    assert(IsLightPosInBounds(light_pos, map_size));

    if (queue->len < queue->cap) {
        queue->buf[(queue->start + queue->len) % queue->cap] = light_pos;
        queue->len++;
        return true;
    }

    return false;
}

static s_vec_2d_i DequeueLightPos(s_light_pos_queue* const queue, const s_vec_2d_i map_size) {
    assert(IsLightPosQueueValid(queue, map_size));
    assert(queue->len > 0);

    const s_vec_2d_i light = queue->buf[queue->start];

    queue->start++;
    queue->start %= queue->cap;

    queue->len--;

    return light;
}

static inline t_light_level LightLevel(const s_lightmap* const map, const s_vec_2d_i pos) {
    assert(IsLightPosInBounds(pos, map->size));
    return map->buf[IndexFrom2D(pos, map->size.x)];
}

static inline void SetLightLevel(const s_lightmap* const map, const s_vec_2d_i pos, const t_light_level lvl) {
    assert(IsLightPosInBounds(pos, map->size));
    assert(IsLightLevelValid(lvl));

    map->buf[IndexFrom2D(pos, map->size.x)] = lvl;
}

s_lightmap GenLightmap(s_mem_arena* const mem_arena, const s_vec_2d_i size, const t_byte* const seed, s_mem_arena* const temp_mem_arena) {
    assert(size.x > 0 && size.y > 0);
    assert(seed);

    const s_lightmap map = {
        .buf = MEM_ARENA_PUSH_TYPE_MANY(mem_arena, t_light_level, size.x * size.y),
        .size = size
    };

    if (!map.buf) {
        fprintf(stderr, "Failed to allocate memory for lightmap!\n");
        return (s_lightmap){0};
    }

    // NOTE: Modelled after BFS.
    const int light_limit = map.size.x * map.size.y;

    s_light_pos_queue queue = {
        .buf = MEM_ARENA_PUSH_TYPE_MANY(temp_mem_arena, s_vec_2d_i, light_limit),
        .cap = light_limit
    };

    if (!queue.buf) {
        fprintf(stderr, "Failed to allocate memory for light position queue!\n");
        return (s_lightmap){0};
    }

    for (int y = 0; y < size.y; y++) {
        for (int x = 0; x < size.x; x++) {
            const s_vec_2d_i pos = {x, y};

            const int i = IndexFrom2D(pos, size.x);

            if (IsBitActive(i, seed, map.size.x * map.size.y)) {
                continue;
            }

            EnqueueLightPos(&queue, pos, map.size);
            SetLightLevel(&map, pos, LIGHT_LEVEL_LIMIT);
        }
    } 

    while (queue.len > 0) {
        const s_vec_2d_i light_pos = DequeueLightPos(&queue, map.size);

        assert(LightLevel(&map, light_pos) > 0); // Sanity check.

        const s_vec_2d_i neighbour_pos_offsets[4] = {
            {0, 1},
            {0, -1},
            {1, 0},
            {-1, 0},
        };

        for (int i = 0; i < STATIC_ARRAY_LEN(neighbour_pos_offsets); i++) {
            const s_vec_2d_i neighbour_pos = Vec2DISum(light_pos, neighbour_pos_offsets[i]);

            if (!IsLightPosInBounds(neighbour_pos, map.size)) {
                continue;
            }

            const t_light_level new_neighbour_light_level = LightLevel(&map, light_pos) - 1;
            assert(IsLightLevelValid(new_neighbour_light_level));

            if (LightLevel(&map, neighbour_pos) >= new_neighbour_light_level) {
                continue;
            }

            SetLightLevel(&map, neighbour_pos, new_neighbour_light_level);

            if (LightLevel(&map, neighbour_pos) > 0) {
                EnqueueLightPos(&queue, neighbour_pos, map.size);
            }
        }
    }

    return map;
}

void RenderLightmap(const s_rendering_context* const rendering_context, const s_lightmap* const map, const s_rect_edges_i range, const float tile_size) {
    assert(tile_size > 0.0f);
    assert(IsRangeValid(range, map->size));

    for (int ty = range.top; ty < range.bottom; ty++) {
        for (int tx = range.left; tx < range.right; tx++) {
            const t_light_level light_lvl = LightLevel(map, (s_vec_2d_i){tx, ty});

            assert(IsLightLevelValid(light_lvl));

            if (light_lvl == LIGHT_LEVEL_LIMIT) {
                continue;
            }

            const s_rect rect = {
                tx * tile_size,
                ty * tile_size,
                tile_size,
                tile_size
            };

            const s_color blend = {
                .a = 1.0f - ((float)light_lvl / LIGHT_LEVEL_LIMIT)
            };

            RenderRect(rendering_context, rect, blend);
        }
    }
}
