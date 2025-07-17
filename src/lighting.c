#include "game.h"

typedef struct {
    s_vec_2d_i* buf;
    int cap;
    int start;
    int len;
} s_light_pos_queue;

static bool IsLightPosInBounds(const s_vec_2d_i pos, const s_vec_2d_i map_size) {
    return pos.x >= 0 && pos.x < map_size.x && pos.y >= 0 && pos.y < map_size.y;
}

static bool IsLightPosQueueValid(const s_light_pos_queue* const queue, const s_vec_2d_i map_size) {
    if (queue->start < 0 || queue->start >= queue->cap || queue->len < 0 || queue->len > queue->cap) {
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

void LoadLightmap(s_lightmap* const map) {
    assert(IS_ZERO(*map));

    // NOTE: Modelled after BFS.
    s_light_pos_queue queue = {0};

    for (int ty = 0; ty < map->size.y / 4; ty++) {
        for (int tx = 0; tx < map->size.x / 4; tx++) {
            EnqueueLightPos(&queue, (s_vec_2d_i){tx, ty}, map->size);
            SetLightLevel(map, (s_vec_2d_i){tx, ty}, LIGHT_LEVEL_LIMIT);
        }
    }

    while (queue.len > 0) {
        const s_vec_2d_i light_pos = DequeueLightPos(&queue, map->size);

        assert(LightLevel(map, light_pos) > 0); // Sanity check.

        const s_vec_2d_i neighbour_pos_offsets[4] = {
            {0, 1},
            {0, -1},
            {1, 0},
            {-1, 0},
        };

        for (int i = 0; i < STATIC_ARRAY_LEN(neighbour_pos_offsets); i++) {
            const s_vec_2d_i neighbour_pos = Vec2DISum(light_pos, neighbour_pos_offsets[i]);

            if (!IsLightPosInBounds(neighbour_pos, map->size)) {
                continue;
            }

            const t_light_level new_neighbour_light_level = LightLevel(map, light_pos) - 1;
            assert(IsLightLevelValid(new_neighbour_light_level));

            if (LightLevel(map, neighbour_pos) >= new_neighbour_light_level) {
                continue;
            }

            SetLightLevel(map, neighbour_pos, new_neighbour_light_level);

            if (LightLevel(map, neighbour_pos) > 0) {
                EnqueueLightPos(&queue, neighbour_pos, map->size);
            }
        }
    }
}

void RenderLightmap(const s_rendering_context* const rendering_context, const s_lightmap* const map, const float tile_size) {
    assert(tile_size > 0.0f);

    for (int ty = 0; ty < map->size.y; ty++) {
        for (int tx = 0; tx < map->size.x; tx++) {
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
