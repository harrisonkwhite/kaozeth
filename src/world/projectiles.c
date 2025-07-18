#include <stdio.h>
#include "world.h"

static inline s_rect ProjectileTranslationCollider(const e_projectile_type proj_type, const s_vec_2d pos_before, const s_vec_2d pos_after) {
    const s_rect colliders[2] = {
        ProjectileCollider(proj_type, pos_before),
        ProjectileCollider(proj_type, pos_after)
    };

    return GenSpanningRect(colliders, 2);
}

s_projectile* SpawnProjectile(s_world* const world, const e_projectile_type type, const bool friendly, const int dmg, const s_vec_2d pos, const s_vec_2d vel) {
    assert(world);
    assert(dmg > 0);

    if (world->proj_cnt == PROJECTILE_LIMIT) {
        fprintf(stderr, "Failed to spawn projectile due to insufficient space!");
        return NULL;
    }

    s_projectile* const proj = &world->projectiles[world->proj_cnt];
    assert(IS_ZERO(*proj));
    proj->type = type;
    proj->friendly = friendly;
    proj->dmg = dmg;
    proj->pos = pos;
    proj->vel = vel;

    world->proj_cnt++;

    return proj;
}

bool UpdateProjectiles(s_world* const world) {
    assert(world);

    // Store the colliders we'll need to test against.
    const s_rect player_collider = PlayerCollider(world->player.pos);

    s_rect npc_colliders[NPC_LIMIT];

    for (int i = 0; i < NPC_LIMIT; i++) {
        if (!IsNPCActive(&world->npcs.activity, i)) {
            continue;
        }

        const s_npc* const npc = &world->npcs.buf[i];
        npc_colliders[i] = NPCCollider(npc->pos, npc->type);
    }

    // Perform an update on each projectile.
    for (int i = 0; i < world->proj_cnt; i++) {
        s_projectile* const proj = &world->projectiles[i]; 

        bool destroy = false; // Do we destroy this projectile at the end of this update?

        const s_projectile_type* const proj_type = &g_projectile_types[proj->type];

        // Perform movement.
        if (proj_type->flags & ek_projectile_type_flags_falls) {
            proj->vel.y += GRAVITY;
        }

        const s_vec_2d pos_before_trans = proj->pos;
        proj->pos = Vec2DSum(proj->pos, proj->vel);

        // Update rotation in accordance with direction if flag is set.
        if (proj_type->flags & ek_projectile_type_flags_rot_is_dir) {
            proj->rot = Dir(proj->vel);
        }

        // Perform collision detection.
        const s_rect proj_trans_collider = ProjectileTranslationCollider(proj->type, pos_before_trans, proj->pos);

        if (proj->friendly) {
            for (int j = 0; j < NPC_LIMIT; j++) {
                if (!IsNPCActive(&world->npcs.activity, j)) {
                    continue;
                }

                s_npc* const npc = &world->npcs.buf[j];
                
                if (DoRectsInters(proj_trans_collider, npc_colliders[j])) {
                    if (!HurtNPC(world, j, proj->dmg, proj->vel)) {
                        return false;
                    }

                    destroy = true;
                }
            }
        } else {
            if (DoRectsInters(proj_trans_collider, player_collider)) {
                if (!HurtPlayer(world, proj->dmg, proj->vel)) {
                    return false;
                }

                destroy = true;
            }
        }

        if (TileCollisionCheck(&world->core.tilemap_core.activity, proj_trans_collider)) {
            destroy = true;
        }

        // Handle destruction.
        if (destroy) {
            // Replace the current projectile with the one at the end.
            world->proj_cnt--;
            world->projectiles[i] = world->projectiles[world->proj_cnt];
            ZERO_OUT(world->projectiles[world->proj_cnt]);
            i--; // Make sure to still update the one at the end we just moved.
        }
    }

    return true;
}

void RenderProjectiles(const s_rendering_context* const rendering_context, const s_projectile* const projectiles, const int proj_cnt, const s_textures* const textures) {
    assert(rendering_context);
    assert(projectiles);
    assert(proj_cnt >= 0);

    for (int i = 0; i < proj_cnt; i++) {
        const s_projectile* const proj = &projectiles[i];
        const s_projectile_type* const proj_type = &g_projectile_types[proj->type];

        RenderSprite(rendering_context, proj_type->spr, textures, proj->pos, (s_vec_2d){0.5f, 0.5f}, (s_vec_2d){1.0f, 1.0f}, proj->rot, WHITE);
    }
}
