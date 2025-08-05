#include "particles.h"

void InitParticleFromTemplate(s_particle* const part, const e_particle_template temp, const s_v2 pos, const s_v2 vel, const float rot) {
    assert(IS_ZERO(*part));

    part->pos = pos;
    part->vel = vel;
    part->rot = rot;

    switch (temp) {
        case ek_particle_template_dirt:
            part->spr = ek_sprite_dirt_particle;
            part->life = ZFW_RandRangeI(20, 25);
            break;

        case ek_particle_template_stone:
            part->spr = ek_sprite_stone_particle;
            part->life = ZFW_RandRangeI(20, 25);
            break;

        case ek_particle_template_grass:
            part->spr = ek_sprite_grass_particle;
            part->life = ZFW_RandRangeI(20, 25);
            break;

        case ek_particle_template_gel:
            part->spr = ek_sprite_gel_particle;
            part->life = ZFW_RandRangeI(20, 25);
            break;

        default:
            assert(false && "Unhandled switch case!");
            break;
    }
}

int AddParticle(s_particles* const particles, const s_particle* const part) {
    assert(part->life > 0);

    if (particles->cnt == PARTICLE_LIMIT) {
        return -1;
    }

    const int index = particles->cnt;
    particles->buf[index] = *part;
    particles->cnt++;
    return index;
}

void UpdateParticles(s_particles* const particles, const float grav) {
    for (int i = 0; i < particles->cnt; i++) {
        s_particle* const part = &particles->buf[i];

        // Update life.
        assert(part->life > 0);

        part->life--;

        if (part->life == 0) {
            particles->cnt--;
            *part = particles->buf[particles->cnt];
        }

        // Update position.
        part->vel.y += grav;
        part->pos = V2Sum(part->pos, part->vel);
    }
}

void RenderParticles(const zfw_s_rendering_context* const rendering_context, const s_particles* const particles, const zfw_s_texture_group* const textures) {
    for (int i = 0; i < particles->cnt; i++) {
        const s_particle* const part = &particles->buf[i];
        RenderSprite(rendering_context, part->spr, textures, part->pos, (s_v2){0.5f, 0.5f}, (s_v2){1.0f, 1.0f}, part->rot, ZFW_WHITE);
    }
}
