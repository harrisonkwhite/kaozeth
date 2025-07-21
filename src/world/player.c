#include <stdio.h>
#include "world.h"

#define PLAYER_MOVE_SPD 1.5f
#define PLAYER_MOVE_SPD_ACC 0.2f
#define PLAYER_JUMP_HEIGHT 3.5f

#define PLAYER_INV_DUR 30
#define PLAYER_INV_ALPHA_LOW 0.5f
#define PLAYER_INV_ALPHA_HIGH 0.7f

static inline bool IsPlayerGrounded(const zfw_s_vec_2d player_pos, const t_tilemap_activity* const tm_activity) {
    const zfw_s_rect below_collider = ZFWRectTranslated(PlayerCollider(player_pos), (zfw_s_vec_2d){0.0f, 1.0f});
    return TileCollisionCheck(tm_activity, below_collider);
}

void InitPlayer(s_player* const player, const int hp_max, const t_tilemap_activity* const tm_activity) {
    assert(player && ZFW_IS_ZERO(*player));
    assert(hp_max >= 0);
    assert(tm_activity);

    player->pos.x = TILE_SIZE * TILEMAP_WIDTH * 0.5f;
    MakeContactWithTilemap(&player->pos, zfw_ek_cardinal_dir_down, PlayerColliderSize(), PLAYER_ORIGIN, tm_activity);

    player->hp = hp_max;
}

void ProcPlayerMovement(s_world* const world, const zfw_s_input_state* const input_state, const zfw_s_input_state* const input_state_last) {
    assert(!world->player.killed);

    const float move_axis = ZFWIsKeyDown(zfw_ek_key_code_d, input_state) - ZFWIsKeyDown(zfw_ek_key_code_a, input_state);
    const float move_spd_dest = move_axis * PLAYER_MOVE_SPD;

    if (world->player.vel.x < move_spd_dest) {
        world->player.vel.x += ZFW_MIN(move_spd_dest - world->player.vel.x, PLAYER_MOVE_SPD_ACC);
    } else if (world->player.vel.x > move_spd_dest) {
        world->player.vel.x -= ZFW_MIN(world->player.vel.x - move_spd_dest, PLAYER_MOVE_SPD_ACC);
    }

    world->player.vel.y += GRAVITY;

    const bool grounded = IsPlayerGrounded(world->player.pos, &world->core.tilemap_core.activity);

    if (grounded) {
        world->player.jumping = false;
    }

    if (!world->player.jumping) {
        if (grounded && ZFWIsKeyPressed(zfw_ek_key_code_space, input_state, input_state_last)) {
            world->player.vel.y = -PLAYER_JUMP_HEIGHT;
            world->player.jumping = true;
        }
    } else {
        if (world->player.vel.y < 0.0f && !ZFWIsKeyDown(zfw_ek_key_code_space, input_state)) {
            world->player.vel.y = 0.0f;
        }
    }

    ProcTileCollisions(&world->player.pos, &world->player.vel, PlayerColliderSize(), PLAYER_ORIGIN, &world->core.tilemap_core.activity);

    world->player.pos = ZFWVec2DSum(world->player.pos, world->player.vel);
}

bool ProcPlayerCollisionsWithNPCs(s_world* const world) {
    assert(!world->player.killed);

    if (world->player.invinc_time > 0) {
        return true;
    }

    const zfw_s_rect player_collider = PlayerCollider(world->player.pos);

    for (int i = 0; i < NPC_LIMIT; i++) {
        const s_npc* const npc = &world->npcs.buf[i]; // NOTE: Constant probably temporarily.

        if (!IsNPCActive(&world->npcs.activity, i)) {
            continue;
        }

        const s_npc_type* const npc_type = &g_npc_types[npc->type];

        if (npc_type->contact_dmg == 0) {
            continue;
        }

        const zfw_s_rect npc_collider = NPCCollider(npc->pos, npc->type);

        if (ZFWDoRectsInters(player_collider, npc_collider)) {
            const zfw_s_vec_2d dir = ZFWVec2DDir(npc->pos, world->player.pos);
            const zfw_s_vec_2d kb = ZFWVec2DScaled(dir, npc_type->contact_kb);

            if (!HurtPlayer(world, npc_type->contact_dmg, kb)) {
                return false;
            }

            break;
        }
    }

    return true;
}

void ProcPlayerDeath(s_world* const world) {
    assert(!world->player.killed);

    if (world->player.hp == 0) {
        // TODO: Do some magic stuff!
        world->player.killed = true;
    }
}

void RenderPlayer(const zfw_s_rendering_context* const rendering_context, const s_world* const world, const zfw_s_textures* const textures) {
    assert(!world->player.killed);

    float alpha = 1.0f;

    if (world->player.invinc_time > 0) {
        alpha = world->player.invinc_time % 2 == 0 ? PLAYER_INV_ALPHA_LOW : PLAYER_INV_ALPHA_HIGH;
    }

    RenderSprite(rendering_context, ek_sprite_player, textures, world->player.pos, PLAYER_ORIGIN, (zfw_s_vec_2d){1.0f, 1.0f}, 0.0f, (zfw_s_color){1.0f, 1.0f, 1.0f, alpha});
}

// Returns true if successful, false otherwise.
bool HurtPlayer(s_world* const world, const int dmg, const zfw_s_vec_2d kb) {
    assert(dmg > 0);
    assert(world->player.invinc_time == 0);

    world->player.hp = ZFW_MAX(world->player.hp - dmg, 0);
    world->player.vel = kb;
    world->player.jumping = false;
    world->player.invinc_time = PLAYER_INV_DUR;

    s_popup_text* const dmg_popup = SpawnPopupText(world, world->player.pos, ZFWRandRange(DMG_POPUP_TEXT_VEL_Y_MIN, DMG_POPUP_TEXT_VEL_Y_MAX));

    if (!dmg_popup) {
        return false;
    }

    snprintf(dmg_popup->str, sizeof(dmg_popup->str), "%d", -dmg);

    return true;
}
