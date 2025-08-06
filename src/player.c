#include "assets.h"
#include "game.h"

#include <stdio.h>

#define PLAYER_MOVE_SPD 1.5f
#define PLAYER_MOVE_SPD_ACC 0.2f
#define PLAYER_JUMP_HEIGHT 3.5f

#define PLAYER_INV_DUR 30
#define PLAYER_INV_ALPHA_LOW 0.5f
#define PLAYER_INV_ALPHA_HIGH 0.7f

static inline bool IsPlayerGrounded(const s_v2 player_pos, const t_tilemap_activity* const tm_activity) {
    const zfw_s_rect below_collider = ZFW_RectTranslated(PlayerCollider(player_pos), (s_v2){0.0f, 1.0f});
    return TileCollisionCheck(tm_activity, below_collider);
}

void InitPlayer(s_player* const player, const int hp_max, const t_tilemap_activity* const tm_activity) {
    assert(player && IS_ZERO(*player));
    assert(hp_max >= 0);
    assert(tm_activity);

    player->pos.x = TILE_SIZE * TILEMAP_WIDTH * 0.5f;
    MakeContactWithTilemap(&player->pos, zfw_ek_cardinal_dir_down, PlayerColliderSize(), PLAYER_ORIGIN, tm_activity);

    player->hp = hp_max;
}

bool UpdatePlayer(s_world* const world, const zfw_s_input_context* const input_context) {
    assert(!world->player.killed);

    //
    // Movement
    //
    const float move_axis = ZFW_IsKeyDown(input_context, zfw_ek_key_code_d) - ZFW_IsKeyDown(input_context, zfw_ek_key_code_a);
    const float move_spd_dest = move_axis * PLAYER_MOVE_SPD;

    if (world->player.vel.x < move_spd_dest) {
        world->player.vel.x += MIN(move_spd_dest - world->player.vel.x, PLAYER_MOVE_SPD_ACC);
    } else if (world->player.vel.x > move_spd_dest) {
        world->player.vel.x -= MIN(world->player.vel.x - move_spd_dest, PLAYER_MOVE_SPD_ACC);
    }

    world->player.vel.y += GRAVITY;

    const bool grounded = IsPlayerGrounded(world->player.pos, &world->core.tilemap_core.activity);

    if (grounded) {
        world->player.jumping = false;
    }

    if (!world->player.jumping) {
        if (grounded && ZFW_IsKeyPressed(input_context, zfw_ek_key_code_space)) {
            world->player.vel.y = -PLAYER_JUMP_HEIGHT;
            world->player.jumping = true;
        }
    } else {
        if (world->player.vel.y < 0.0f && !ZFW_IsKeyDown(input_context, zfw_ek_key_code_space)) {
            world->player.vel.y = 0.0f;
        }
    }

    ProcTileCollisions(&world->player.pos, &world->player.vel, PlayerColliderSize(), PLAYER_ORIGIN, &world->core.tilemap_core.activity);

    world->player.pos = V2Sum(world->player.pos, world->player.vel);

    //
    // NPC Collisions
    //
    if (world->player.invinc_time <= 0) {
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

            if (ZFW_DoRectsInters(player_collider, npc_collider)) {
                const s_v2 dir = V2Dir(npc->pos, world->player.pos);
                const s_v2 kb = {dir.x * npc_type->contact_kb, dir.y * npc_type->contact_kb};

                if (!HurtPlayer(world, npc_type->contact_dmg, kb)) {
                    return false;
                }

                break;
            }
        }
    }

    //
    // Updating Timers
    //
    if (world->player.invinc_time > 0) {
        world->player.invinc_time--;
    }

    if (world->player.flash_time > 0) {
        world->player.flash_time--;
    }

    //
    // Death
    //
    if (world->player.hp == 0) {
        // TODO: Do some magic stuff!
        world->player.killed = true;
    }

    return true;
}

void RenderPlayer(const s_player* const player, const zfw_s_rendering_context* const rendering_context, const zfw_s_texture_group* const textures, const zfw_s_shader_prog_group* const shader_progs, const zfw_s_surface_group* const surfs) {
    assert(!player->killed);

    float alpha = 1.0f;

    if (player->invinc_time > 0) {
        alpha = player->invinc_time % 2 == 0 ? PLAYER_INV_ALPHA_LOW : PLAYER_INV_ALPHA_HIGH;
    }

    if (player->flash_time > 0) {
        ZFW_SetSurface(rendering_context, surfs, ek_surface_temp);
        ZFW_Clear(rendering_context, (u_v4){0});
    }

    RenderSprite(rendering_context, ek_sprite_player, textures, player->pos, PLAYER_ORIGIN, (s_v2){1.0f, 1.0f}, 0.0f, (u_v4){1.0f, 1.0f, 1.0f, alpha});

    if (player->flash_time > 0) {
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

bool HurtPlayer(s_world* const world, const int dmg, const s_v2 kb) {
    assert(dmg > 0);
    assert(world->player.invinc_time == 0);

    world->player.hp = MAX(world->player.hp - dmg, 0);
    world->player.vel = kb;
    world->player.jumping = false;
    world->player.invinc_time = PLAYER_INV_DUR;
    world->player.flash_time = PLAYER_HURT_FLASH_TIME;

    s_popup_text* const dmg_popup = SpawnPopupText(world, world->player.pos, ZFW_RandRange(DMG_POPUP_TEXT_VEL_Y_MIN, DMG_POPUP_TEXT_VEL_Y_MAX));

    if (!dmg_popup) {
        return false;
    }

    snprintf(dmg_popup->str, sizeof(dmg_popup->str), "%d", -dmg);

    return true;
}
