#include "game.h"

#include <stdio.h>

#define NPC_ORIGIN (s_vec_2d){0.5f, 0.5f}

static void SlimeNPCTick(s_world* const world, const int npc_index) {
    assert(world);
    assert(npc_index >= 0 && npc_index < NPC_LIMIT);
    assert(IsNPCActive(&world->npcs.activity, npc_index));

    s_npc* const npc = &world->npcs.buf[npc_index];
    assert(npc->type == ek_npc_type_slime);

    s_slime_npc* const slime = &npc->type_data.slime;

    npc->vel.y += GRAVITY;

    // TEMP: Slime should only jump when grounded, and at a more random interval.
    if (slime->jump_time < 60) {
        slime->jump_time++;
    } else {
        npc->vel.y = -4.0f;
        slime->jump_time = 0;
    }

    const s_rect collider = NPCCollider(npc->pos, npc->type);
    ProcTileCollisions(&npc->vel, collider, &world->tilemap.activity);

    npc->pos = Vec2DSum(npc->pos, npc->vel);
}

const s_npc_type g_npc_types[eks_npc_type_cnt] = {
    [ek_npc_type_slime] = {
        .name = "Slime",
        .spr = ek_sprite_slime,
        .tick_func = SlimeNPCTick,
        .contact_dmg = 8,
        .contact_kb = 5.0f
    }
};

static_assert(sizeof(g_npc_types) == sizeof(s_npc_type) * eks_npc_type_cnt, "Invalid array length!");

int SpawnNPC(s_npcs* const npcs, const s_vec_2d pos, const e_npc_type type) {
    const int index = FirstInactiveBitIndex(npcs->activity, NPC_LIMIT);

    if (index != -1) {
        ActivateBit(index, npcs->activity, NPC_LIMIT);

        s_npc* const npc = &npcs->buf[index];
        assert(IsZero(npc, sizeof(*npc))); // Should have been cleared when the NPC slot was deactivated.

        npc->pos = pos;
        npc->type = type;
    } else {
        fprintf(stderr, "Failed to spawn NPC due to insufficient space!\n");
    }

    return index;
}

void RunNPCTicks(s_world* const world) {
    for (int i = 0; i < NPC_LIMIT; i++) {
        if (!IsNPCActive(&world->npcs.activity, i)) {
            continue;
        }

        s_npc* const npc = &world->npcs.buf[i];
        const s_npc_type npc_type = g_npc_types[npc->type];
        npc_type.tick_func(world, i);
    }
}

void RenderNPCs(const s_rendering_context* const rendering_context, const s_npcs* const npcs, const s_textures* const textures) {
    for (int i = 0; i < NPC_LIMIT; i++) {
        if (!IsNPCActive(&npcs->activity, i)) {
            continue;
        }

        const s_npc* const npc = &npcs->buf[i];
        const e_sprite spr = g_npc_types[npc->type].spr;

        RenderSprite(rendering_context, spr, textures, npc->pos, (s_vec_2d){0.5f, 0.5f}, (s_vec_2d){1.0f, 1.0f}, 0.0f, WHITE);
    }
}

s_rect NPCCollider(const s_vec_2d npc_pos, const e_npc_type npc_type) {
    return ColliderFromSprite(g_npc_types[npc_type].spr, npc_pos, NPC_ORIGIN);
}

bool IsNPCActive(const t_npc_activity* const activity, const int index) {
    assert(activity);
    assert(index >= 0 && index < NPC_LIMIT);
    return IsBitActive(index, (t_byte*)activity, NPC_LIMIT);
}
