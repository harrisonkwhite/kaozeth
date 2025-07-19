#ifndef PARTICLES_H
#define PARTICLES_H

#include <zfw_rendering.h>
#include <zfw_math.h>
#include "sprites.h"

#define PARTICLE_LIMIT 1024

typedef struct {
    s_vec_2d pos;
    s_vec_2d vel;
    float vel_mult;
} s_particle_pos_info;

typedef struct {
    float rot;
    float rot_change;
    float rot_change_mult;
} s_particle_rot_info;

typedef enum {
    ek_particle_template_dirt,
    ek_particle_template_stone,
    ek_particle_template_grass,
    ek_particle_template_gel,

    eks_particle_template_cnt
} e_particle_template;

typedef struct {
    s_particle_pos_info pos_infos[PARTICLE_LIMIT];
    s_particle_rot_info rot_infos[PARTICLE_LIMIT];
    e_sprite sprites[PARTICLE_LIMIT];

    int cnt;
} s_particles;

int SpawnParticle(s_particles* const particles, const s_vec_2d pos, const s_vec_2d vel, const e_sprite spr);
int SpawnParticleFromTemplate(s_particles* const particles, const s_vec_2d pos, const s_vec_2d vel, const e_particle_template template);
void UpdateParticles(s_particles* const particles, const float grav);
void RenderParticles(const s_rendering_context* const rendering_context, const s_particles* const particles, const s_textures* const textures);

#endif
