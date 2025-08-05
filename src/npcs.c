#include "assets.h"
#include "game.h"

#include <stdio.h>

#define SLIME_VEL_X_LERP 0.2f
#define SLIME_JUMP_HEIGHT_MIN 3.0f
#define SLIME_JUMP_HEIGHT_MAX 4.0f
#define SLIME_JUMP_TIME_MIN 50
#define SLIME_JUMP_TIME_MAX 120
#define SLIME_JUMP_HOR_SPD_MIN 1.0f
#define SLIME_JUMP_HOR_SPD_MAX 2.0f

static inline bool IsNPCOnGround(const s_v2 npc_pos, const e_npc_type npc_type, const t_tilemap_activity* const tilemap_activity) {
    assert(tilemap_activity);

    const zfw_s_rect collider = NPCCollider(npc_pos, npc_type);
    const zfw_s_rect below_collider = ZFW_RectTranslated(collider, (s_v2){0.0f, 1.0f});
    return TileCollisionCheck(tilemap_activity, below_collider);
}

static void SlimeNPCTick(s_world* const world, const int npc_index) {
    assert(world);
    assert(npc_index >= 0 && npc_index < NPC_LIMIT);
    assert(IsNPCActive(&world->npcs.activity, npc_index));

    s_npc* const npc = &world->npcs.buf[npc_index];
    assert(npc->type == ek_npc_type_slime);

    s_slime_npc* const slime = &npc->type_data.slime;

    npc->vel.y += GRAVITY;

    float vel_x_dest = 0.0f;

    if (IsNPCOnGround(npc->pos, npc->type, &world->core.tilemap_core.activity)) {
        if (slime->jump_time > 0) {
            slime->jump_time--;
        } else {
            npc->vel.y = -ZFW_RandRange(SLIME_JUMP_HEIGHT_MIN, SLIME_JUMP_HEIGHT_MAX);
            slime->jump_time = ZFW_RandRangeI(SLIME_JUMP_TIME_MIN, SLIME_JUMP_TIME_MAX);
            slime->jump_hor_spd = ZFW_RandRange(SLIME_JUMP_HOR_SPD_MIN, SLIME_JUMP_HOR_SPD_MAX) * SIGN(world->player.pos.x - npc->pos.x);
        }
    } else {
        vel_x_dest = slime->jump_hor_spd;
    }

    npc->vel.x = Lerp(npc->vel.x, vel_x_dest, SLIME_VEL_X_LERP);

    ProcTileCollisions(&npc->pos, &npc->vel, NPCColliderSize(npc->type), NPC_ORIGIN, &world->core.tilemap_core.activity);

    npc->pos = V2Sum(npc->pos, npc->vel);
}

static void PostSlimeNPCSpawn(s_world* const world, const int npc_index) {
    assert(world);
    assert(npc_index >= 0 && npc_index < NPC_LIMIT);
    assert(IsNPCActive(&world->npcs.activity, npc_index));

    s_npc* const npc = &world->npcs.buf[npc_index];
    assert(npc->type == ek_npc_type_slime);

    s_slime_npc* const slime = &npc->type_data.slime;
    slime->jump_time = ZFW_RandRangeI(SLIME_JUMP_TIME_MIN, SLIME_JUMP_TIME_MAX);
}

const s_npc_type g_npc_types[eks_npc_type_cnt] = {
    [ek_npc_type_slime] = {
        .name = "Slime",
        .spr = ek_sprite_slime,
        .tick_func = SlimeNPCTick,
        .postspawn_func = PostSlimeNPCSpawn,
        .hp_max = 10,
        .contact_dmg = 8,
        .contact_kb = 4.0f
    }
};

static_assert(sizeof(g_npc_types) == sizeof(s_npc_type) * eks_npc_type_cnt, "Invalid array length!");

static bool IsNPCValid(const s_npc* const npc) {
    assert(npc);
    return npc->type >= 0 && npc->type < eks_npc_type_cnt && npc->hp >= 0 && npc->hp <= g_npc_types[npc->type].hp_max;
}

int SpawnNPC(s_world* const world, const s_v2 pos, const e_npc_type type, const t_tilemap_activity* const tm_activity) {
    const int index = FirstInactiveBitIndex(world->npcs.activity, NPC_LIMIT);

    if (index != -1) {
        ActivateBit(world->npcs.activity, index, NPC_LIMIT);

        s_npc* const npc = &world->npcs.buf[index];
        assert(IS_ZERO(*npc)); // Should have been cleared when the NPC slot was deactivated.

        npc->pos = pos;
        npc->hp = g_npc_types[type].hp_max;
        npc->type = type;

        MakeContactWithTilemap(&npc->pos, zfw_ek_cardinal_dir_down, NPCColliderSize(npc->type), NPC_ORIGIN, tm_activity); // Grounds the NPC.

        if (g_npc_types[type].postspawn_func) {
            g_npc_types[type].postspawn_func(world, index);
        }
    } else {
        fprintf(stderr, "Failed to spawn NPC due to insufficient space!\n");
    }

    return index;
}

void UpdateNPCs(s_world* const world) {
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

void ProcNPCDeaths(s_world* const world) {
    assert(world);

    for (int i = 0; i < NPC_LIMIT; i++) {
        if (!IsNPCActive(&world->npcs.activity, i)) {
            continue;
        }

        s_npc* const npc = &world->npcs.buf[i];
        assert(IsNPCValid(npc));

        if (npc->hp == 0) {
            DeactivateBit(world->npcs.activity, i, NPC_LIMIT);
            ZERO_OUT(*npc);
        }
    }
}

void RenderNPCs(const s_npcs* const npcs, const zfw_s_rendering_context* const rendering_context, const zfw_s_texture_group* const textures, const zfw_s_shader_prog_group* const shader_progs, const zfw_s_surface_group* const surfs) {
    for (int i = 0; i < NPC_LIMIT; i++) {
        if (!IsNPCActive(&npcs->activity, i)) {
            continue;
        }

        const s_npc* const npc = &npcs->buf[i];
        const e_sprite spr = g_npc_types[npc->type].spr;

        if (npc->flash_time > 0) {
            ZFW_SetSurface(rendering_context, surfs, ek_surface_temp);
            ZFW_Clear(rendering_context, (u_v4){0});
        }

        RenderSprite(rendering_context, spr, textures, npc->pos, NPC_ORIGIN, (s_v2){1.0f, 1.0f}, 0.0f, ZFW_WHITE);

        if (npc->flash_time > 0) {
            ZFW_UnsetSurface(rendering_context);

            ZFW_SetSurfaceShaderProg(rendering_context, shader_progs, ek_shader_prog_blend);

            const zfw_s_shader_prog_uniform_value col_uni_val = {
                .type = zfw_ek_shader_prog_uniform_value_type_v3,
                .as_v3 = {1.0f, 1.0f, 1.0f}
            };

            ZFW_SetSurfaceShaderProgUniform(rendering_context, "u_col", col_uni_val);
            ZFW_RenderSurface(rendering_context, surfs, ek_surface_temp);
        }
    }
}

// Returns true if successful, false otherwise.
bool HurtNPC(s_world* const world, const int npc_index, const int dmg, const s_v2 kb) {
    assert(world);
    assert(npc_index >= 0 && npc_index < NPC_LIMIT);
    assert(IsNPCActive(&world->npcs.activity, npc_index));
    assert(dmg > 0);

    s_npc* const npc = &world->npcs.buf[npc_index];
    npc->hp = MAX(npc->hp - dmg, 0);
    npc->vel = kb;
    npc->flash_time = NPC_HURT_FLASH_TIME;

    s_popup_text* const dmg_popup = SpawnPopupText(world, npc->pos, ZFW_RandRange(DMG_POPUP_TEXT_VEL_Y_MIN, DMG_POPUP_TEXT_VEL_Y_MAX));

    if (!dmg_popup) {
        return false;
    }

    snprintf(dmg_popup->str, sizeof(dmg_popup->str), "%d", -dmg);

    return true;
}

bool IsNPCActive(const t_npc_activity* const activity, const int index) {
    assert(activity);
    assert(index >= 0 && index < NPC_LIMIT);
    return IsBitActive((const t_byte*)activity, index, NPC_LIMIT);
}

static int NPCCnt(const t_npc_activity* const activity) {
    int cnt = 0;

    for (int i = 0; i < NPC_LIMIT; i++) {
        if (IsNPCActive(activity, i)) {
            cnt++;
        }
    }

    return cnt;
}

static float GenEnemySpawnX(const float cam_x, const float cam_width, const float offs, const float max_innerness) {
    assert(cam_width > 0.0f);
    assert(offs >= 0.0f);
    assert(max_innerness <= cam_width / 2.0f);

    const float cam_left = cam_x - (cam_width / 2.0f);
    const float cam_right = cam_x + (cam_width / 2.0f);
    const float left = cam_left - offs;
    const float right = cam_right + offs;

    float x;

    do {
        x = ZFW_RandRange(left, right);
    } while (x >= cam_left + max_innerness && x < cam_right - max_innerness);

    return CLAMP(x, 0.0f, WORLD_WIDTH - 1.0f);
}

bool ProcEnemySpawning(s_world* const world, const float cam_width) {
    assert(cam_width > 0.0f);

    const float spawn_rate = 0.004f;
    const int spawn_limit = 4;

    if (ZFW_RandPerc() < spawn_rate) {
        const int npc_cnt = NPCCnt(&world->npcs.activity);

        if (npc_cnt < spawn_limit) {
            const float spawn_x = GenEnemySpawnX(world->cam.pos.x, cam_width, cam_width, -(cam_width / 4.0f));

            if (SpawnNPC(world, (s_v2){spawn_x, 0.0f}, ek_npc_type_slime, &world->core.tilemap_core.activity) == -1) {
                return false;
            }
        }
    }

    return true;
}
