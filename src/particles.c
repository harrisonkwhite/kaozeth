#include "particles.h"

int SpawnParticle(s_particles* const particles, const s_vec_2d pos, const s_vec_2d vel, const e_sprite spr) {
    if (particles->cnt == PARTICLE_LIMIT) {
        return -1;
    }

    const int index = particles->cnt;

    particles->pos_infos[index] = (s_particle_pos_info){
        .pos = pos,
        .vel = vel
    };

    particles->sprites[index] = spr;

    particles->cnt++;

    return index;
}

int SpawnParticleFromTemplate(s_particles* const particles, const s_vec_2d pos, const s_vec_2d vel, const e_particle_template template) {
    const int index = SpawnParticle(particles, pos, vel, ek_sprite_dirt_tile); // TEMP

    if (index != -1) {
        switch (template) {
            case ek_particle_template_dirt:
                break;
        }
    }

    return index;
}

void UpdateParticles(s_particles* const particles) {
    // Update positions.
    for (int i = 0; i < particles->cnt; i++) {
        s_particle_pos_info* const pos_info = &particles->pos_infos[i];
        pos_info->pos = Vec2DSum(pos_info->pos, pos_info->vel);
        pos_info->vel = Vec2DScaled(pos_info->vel, pos_info->vel_mult);
    }

    // Update rotations.
    for (int i = 0; i < particles->cnt; i++) {
        s_particle_rot_info* const rot_info = &particles->rot_infos[i];
        rot_info->rot += rot_info->rot_change;
        rot_info->rot_change *= rot_info->rot_change_mult;
    }
}

void RenderParticles(const s_rendering_context* const rendering_context, const s_particles* const particles, const s_textures* const textures) {
    for (int i = 0; i < particles->cnt; i++) {
        const s_vec_2d pos = particles->pos_infos[i].pos;
        const float rot = particles->rot_infos[i].rot;
        const s_vec_2d origin = {0.5f, 0.5f};

        RenderSprite(rendering_context, particles->sprites[i], textures, pos, origin, (s_vec_2d){1.0f, 1.0f}, rot, WHITE);
    }
}
