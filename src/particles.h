#ifndef PARTICLES_H
#define PARTICLES_H

#include <zfw.h>
#include "sprites.h"

#define PARTICLE_LIMIT 1024

typedef struct {
    e_sprite spr; // NOTE: Not needed during tick!

    s_v2 pos;
    s_v2 vel;

    float rot;

    int life;
} s_particle;

typedef struct {
    s_particle buf[PARTICLE_LIMIT];
    int cnt;
} s_particles;

typedef enum {
    ek_particle_template_dirt,
    ek_particle_template_stone,
    ek_particle_template_grass,
    ek_particle_template_gel,

    eks_particle_template_cnt
} e_particle_template;

void InitParticleFromTemplate(s_particle* const part, const e_particle_template temp, const s_v2 pos, const s_v2 vel, const float rot);
int AddParticle(s_particles* const particles, const s_particle* const part);
void UpdateParticles(s_particles* const particles, const float grav);
void RenderParticles(const zfw_s_rendering_context* const rendering_context, const s_particles* const particles, const zfw_s_texture_group* const textures);

static inline int SpawnParticleFromTemplate(s_particles* const particles, const e_particle_template temp, const s_v2 pos, const s_v2 vel, const float rot) {
    s_particle part = {0};
    InitParticleFromTemplate(&part, temp, pos, vel, rot);
    return AddParticle(particles, &part);
}

#endif
