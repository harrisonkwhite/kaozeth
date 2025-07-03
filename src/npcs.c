#include <stdio.h>
#include <zfw_random.h>
#include "game.h"

#define NPC_ORIGIN (s_vec_2d){0.5f, 0.5f}

#define SLIME_VEL_X_LERP 0.2f
#define SLIME_AERIAL_HOR_MOVE_SPD 1.5f

static inline bool IsNPCOnGround(const s_vec_2d npc_pos, const e_npc_type npc_type, const t_tilemap_activity* const tilemap_activity) {
    assert(tilemap_activity);

    const s_rect collider = NPCCollider(npc_pos, npc_type);
    const s_rect below_collider = RectTranslated(collider, (s_vec_2d){0.0f, 1.0f});
    return TileCollisionCheck(tilemap_activity, below_collider);
}

static void SlimeNPCTick(s_world_state* const world, const int npc_index) {
    assert(world);
    assert(npc_index >= 0 && npc_index < NPC_LIMIT);
    assert(IsNPCActive(&world->npcs.activity, npc_index));

    s_npc* const npc = &world->npcs.buf[npc_index];
    assert(npc->type == ek_npc_type_slime);

    s_slime_npc* const slime = &npc->type_data.slime;

    npc->vel.y += GRAVITY;

    float vel_x_dest = 0.0f;

    if (IsNPCOnGround(npc->pos, npc->type, &world->tilemap.activity)) {
        if (slime->jump_time < 60) {
            slime->jump_time++;
        } else {
            npc->vel.y = -4.0f;
            slime->jump_time = 0;
            slime->jump_hor_sign = SIGN(world->player.pos.x - npc->pos.x);
        }
    } else {
        vel_x_dest = slime->jump_hor_sign * SLIME_AERIAL_HOR_MOVE_SPD;
    }

    npc->vel.x = Lerp(npc->vel.x, vel_x_dest, SLIME_VEL_X_LERP);

    const s_rect collider = NPCCollider(npc->pos, npc->type);
    ProcTileCollisions(&npc->vel, collider, &world->tilemap.activity);

    npc->pos = Vec2DSum(npc->pos, npc->vel);
}

const s_npc_type g_npc_types[eks_npc_type_cnt] = {
    [ek_npc_type_slime] = {
        .name = "Slime",
        .spr = ek_sprite_slime,
        .tick_func = SlimeNPCTick,
        .hp_max = 10,
        .contact_dmg = 8,
        .contact_kb = 5.0f
    }
};

static_assert(sizeof(g_npc_types) == sizeof(s_npc_type) * eks_npc_type_cnt, "Invalid array length!");

static bool IsNPCValid(const s_npc* const npc) {
    assert(npc);
    return npc->type >= 0 && npc->type < eks_npc_type_cnt && npc->hp >= 0 && npc->hp <= g_npc_types[npc->type].hp_max;
}

int SpawnNPC(s_npcs* const npcs, const s_vec_2d pos, const e_npc_type type) {
    const int index = FirstInactiveBitIndex(npcs->activity, NPC_LIMIT);

    if (index != -1) {
        ActivateBit(index, npcs->activity, NPC_LIMIT);

        s_npc* const npc = &npcs->buf[index];
        assert(IS_ZERO(*npc)); // Should have been cleared when the NPC slot was deactivated.

        npc->pos = pos;
        npc->hp = g_npc_types[type].hp_max;
        npc->type = type;
    } else {
        fprintf(stderr, "Failed to spawn NPC due to insufficient space!\n");
    }

    return index;
}

void RunNPCTicks(s_world_state* const world) {
    assert(world);

    for (int i = 0; i < NPC_LIMIT; i++) {
        if (!IsNPCActive(&world->npcs.activity, i)) {
            continue;
        }

        s_npc* const npc = &world->npcs.buf[i];
        assert(IsNPCValid(npc));
        const s_npc_type npc_type = g_npc_types[npc->type];
        npc_type.tick_func(world, i);
    }
}

void ProcNPCDeaths(s_world_state* const world) {
    assert(world);

    for (int i = 0; i < NPC_LIMIT; i++) {
        if (!IsNPCActive(&world->npcs.activity, i)) {
            continue;
        }

        s_npc* const npc = &world->npcs.buf[i];
        assert(IsNPCValid(npc));

        if (npc->hp == 0) {
            DeactivateBit(i, world->npcs.activity, NPC_LIMIT);
        }
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

// Returns true if successful, false otherwise.
bool HurtNPC(s_world_state* const world, const int npc_index, const int dmg, const s_vec_2d kb) {
    assert(world);
    assert(npc_index >= 0 && npc_index < NPC_LIMIT);
    assert(IsNPCActive(&world->npcs.activity, npc_index));
    assert(dmg > 0);

    s_npc* const npc = &world->npcs.buf[npc_index];
    npc->hp = MAX(npc->hp - dmg, 0);
    npc->vel = Vec2DSum(npc->vel, kb);

    s_popup_text* const dmg_popup = SpawnPopupText(world, npc->pos, RandRange(DMG_POPUP_TEXT_VEL_Y_MIN, DMG_POPUP_TEXT_VEL_Y_MAX));

    if (!dmg_popup) {
        return false;
    }

    snprintf(dmg_popup->str, sizeof(dmg_popup->str), "%d", -dmg);

    return true;
}

s_rect NPCCollider(const s_vec_2d npc_pos, const e_npc_type npc_type) {
    return ColliderFromSprite(g_npc_types[npc_type].spr, npc_pos, NPC_ORIGIN);
}

bool IsNPCActive(const t_npc_activity* const activity, const int index) {
    assert(activity);
    assert(index >= 0 && index < NPC_LIMIT);
    return IsBitActive(index, (t_byte*)activity, NPC_LIMIT);
}
