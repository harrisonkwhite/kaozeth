#include "game.h"

#define PLAYER_ORIGIN (s_vec_2d){0.5f, 0.5f}

void ProcPlayerMovement(s_world* const world, const s_input_state* const input_state, const s_input_state* const input_state_last) {
    const float move_axis = IsKeyDown(ek_key_code_d, input_state) - IsKeyDown(ek_key_code_a, input_state);
    const float move_spd_dest = move_axis * PLAYER_MOVE_SPD;
    world->player_vel.x = Lerp(world->player_vel.x, move_spd_dest, PLAYER_MOVE_SPD_LERP);

    world->player_vel.y += GRAVITY;

    if (!world->player_jumping) {
        if (IsKeyPressed(ek_key_code_space, input_state, input_state_last)) {
            world->player_vel.y = -PLAYER_JUMP_HEIGHT;
            world->player_jumping = true;
        }
    } else {
        if (world->player_vel.y < 0.0f && !IsKeyDown(ek_key_code_space, input_state)) {
            world->player_vel.y = 0.0f;
        }
    }

    {
        const s_rect collider = PlayerCollider(world->player_pos);
        ProcTileCollisions(&world->player_vel, collider, &world->tilemap_activity);
    }

    world->player_pos = Vec2DSum(world->player_pos, world->player_vel);

    // Leave jumping state if tile is below.
    const s_rect below_collider = RectTranslated(PlayerCollider(world->player_pos), (s_vec_2d){0.0f, 1.0f});

    if (TileCollisionCheck(&world->tilemap_activity, below_collider)) {
        world->player_jumping = false;
    }
}

void RenderPlayer(const s_rendering_context* const rendering_context, const s_vec_2d player_pos, const s_textures* const textures) {
    RenderSprite(rendering_context, ek_sprite_player, textures, player_pos, PLAYER_ORIGIN, (s_vec_2d){1.0f, 1.0f}, 0.0f, WHITE);
}

s_rect PlayerCollider(const s_vec_2d pos) {
    return ColliderFromSprite(ek_sprite_player, pos, PLAYER_ORIGIN);
}
