#include <stdio.h>
#include "game.h"

const s_projectile_type g_projectile_types[] = {
    [ek_projectile_type_wooden_arrow] = {
        .spr = ek_sprite_projectile
    }
};

static_assert(STATIC_ARRAY_LEN(g_projectile_types) == eks_projectile_type_cnt, "Invalid array length!");

static void DestroyProjectile(s_world* const world, const int index) {
    assert(world);
    assert(index >= 0 && index < world->proj_cnt);

    world->proj_cnt--;
    world->projectiles[index] = world->projectiles[world->proj_cnt];
}

s_projectile* SpawnProjectile(s_world* const world, const e_projectile_type type, const s_vec_2d pos, const s_vec_2d vel) {
    assert(world);

    if (world->proj_cnt == PROJECTILE_LIMIT) {
        fprintf(stderr, "Failed to spawn projectile due to insufficient space!");
        return NULL;
    }

    s_projectile* const proj = &world->projectiles[world->proj_cnt];
    proj->pos = pos;
    proj->vel = vel;

    world->proj_cnt++;

    return proj;
}

void UpdateProjectiles(s_world* const world) {
    assert(world);

    for (int i = 0; i < world->proj_cnt; i++) {
        s_projectile* const proj = &world->projectiles[i]; 

        const s_projectile_type* const proj_type = &g_projectile_types[proj->type];

        if (proj_type->flags & ek_projectile_type_flags_falls) {
            proj->vel.y += GRAVITY;
        }

        proj->pos = Vec2DSum(proj->pos, proj->vel);
    }
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
