#include <stdio.h>
#include "world.h"
#include "zfw_math.h"
#include "zfw_random.h"
#include "zfw_utils.h"

#define RESPAWN_TIME 120

static void InitCameraViewMatrix(zfw_t_matrix_4x4* const mat, const s_camera* const cam, const zfw_s_vec_2d_i window_size) {
    assert(mat && ZFW_IS_ZERO(*mat));
    assert(window_size.x > 0 && window_size.y > 0);

    const zfw_s_vec_2d view_pos = {
        (-cam->pos.x * cam->scale) + (window_size.x / 2.0f),
        (-cam->pos.y * cam->scale) + (window_size.y / 2.0f)
    };

    ZFWInitIdenMatrix4x4(mat);
    ZFWTranslateMatrix4x4(mat, view_pos);
    ZFWScaleMatrix4x4(mat, cam->scale);
}

static inline float CalcCameraScale(const zfw_s_vec_2d_i display_size) {
    return display_size.x > 1600 || display_size.y > 900 ? 3.0f : 2.0f;
}

bool InitWorld(s_world* const world, const t_world_filename* const filename, const zfw_s_vec_2d_i window_size, zfw_s_mem_arena* const temp_mem_arena) {
    assert(world && ZFW_IS_ZERO(*world));
    assert(filename);

    if (!ZFWInitMemArena(&world->mem_arena, WORLD_MEM_ARENA_SIZE)) {
        fprintf(stderr, "Failed to initialise world memory arena!\n");
        return false;
    }

    if (!LoadWorldCoreFromFile(&world->core, filename)) {
        return false;
    }

    InitPlayer(&world->player, world->core.player_hp_max, &world->core.tilemap_core.activity);

    world->cam = (s_camera){
        .pos = world->player.pos,
        .scale = CalcCameraScale(window_size)
    };

    AddToInventory((s_inventory_slot*)world->player_inv_slots, PLAYER_INVENTORY_LEN, ek_item_type_copper_pickaxe, 1);

    return true;
}

void CleanWorld(s_world* const world) {
    ZFWCleanMemArena(&world->mem_arena);
}

bool WorldTick(s_world* const world, const t_settings* const settings, const zfw_s_input_state* const input_state, const zfw_s_input_state* const input_state_last, const zfw_s_vec_2d_i window_size, zfw_s_audio_sys* const audio_sys, const zfw_s_sound_types* const snd_types) {
    assert(window_size.x > 0 && window_size.y > 0); 

    world->cam.scale = CalcCameraScale(window_size);
    const zfw_s_vec_2d cam_size = CameraSize(world->cam.scale, window_size);

    if (!world->player.killed) {
        ProcPlayerMovement(world, input_state, input_state_last);
        ProcPlayerCollisionsWithNPCs(world);

        if (world->player.invinc_time > 0) {
            world->player.invinc_time--;
        }

        ProcPlayerDeath(world);
    }

    if (world->player.killed) {
        if (world->respawn_time < RESPAWN_TIME) {
            world->respawn_time++;
        } else {
            world->respawn_time = 0;

            ZFW_ZERO_OUT(world->player);
            InitPlayer(&world->player, world->core.player_hp_max, &world->core.tilemap_core.activity);
        }
    }

    if (!ProcEnemySpawning(world, cam_size.x)) {
        return false;
    }

    UpdateNPCs(world);
    ProcNPCDeaths(world); // NOTE: Might need to defer this until later in the tick.

    if (!UpdateItemDrops(world, audio_sys, snd_types, settings)) {
        return false;
    }

    if (!ProcItemUsage(world, input_state, window_size)) {
        return false;
    }

    if (!UpdateProjectiles(world)) {
        return false;
    }

    UpdateWorldUI(world, input_state, input_state_last, window_size);

    // Update the camera.
    {
        const zfw_s_vec_2d cam_pos_dest = world->player.pos;

        if (SettingToggle(settings, ek_setting_smooth_camera)) {
            world->cam.pos = ZFWLerpVec2D(world->cam.pos, cam_pos_dest, CAMERA_LERP);

            world->cam.pos = (zfw_s_vec_2d){
                ZFW_CLAMP(world->cam.pos.x, cam_size.x / 2.0f, WORLD_WIDTH - (cam_size.x / 2.0f)),
                ZFW_CLAMP(world->cam.pos.y, cam_size.y / 2.0f, WORLD_HEIGHT - (cam_size.y / 2.0f))
            };
        } else {
            world->cam.pos = cam_pos_dest;
        }
    }

    UpdateParticles(&world->particles, GRAVITY);

    return true;
}

static zfw_s_rect_edges_i TilemapRenderRange(const s_camera* const cam, const zfw_s_vec_2d_i window_size) {
    assert(window_size.x > 0 && window_size.y > 0);

    const zfw_s_vec_2d cam_tl = CameraTopLeft(cam, window_size);
    const zfw_s_vec_2d cam_size = CameraSize(cam->scale, window_size);

    zfw_s_rect_edges_i render_range = {
        .left = floorf(cam_tl.x / TILE_SIZE),
        .top = floorf(cam_tl.y / TILE_SIZE),
        .right = ceilf((cam_tl.x + cam_size.x) / TILE_SIZE),
        .bottom = ceilf((cam_tl.y + cam_size.y) / TILE_SIZE)
    };

    // Clamp the tilemap render range within tilemap bounds.
    render_range.left = ZFW_CLAMP(render_range.left, 0, TILEMAP_WIDTH - 1);
    render_range.top = ZFW_CLAMP(render_range.top, 0, TILEMAP_HEIGHT - 1);
    render_range.right = ZFW_CLAMP(render_range.right, 0, TILEMAP_WIDTH);
    render_range.bottom = ZFW_CLAMP(render_range.bottom, 0, TILEMAP_HEIGHT);

    return render_range;
}

static s_lightmap GenWorldLightmap(zfw_s_mem_arena* const mem_arena, const t_tilemap_activity* const tm_activity, const zfw_s_rect_edges_i tm_render_range, zfw_s_mem_arena* const temp_mem_arena) {
    const s_lightmap lightmap = GenLightmap(mem_arena, (zfw_s_vec_2d_i){tm_render_range.right - tm_render_range.left, tm_render_range.bottom - tm_render_range.top});

    if (!IsLightmapInitted(&lightmap)) {
        return (s_lightmap){0};
    }

    for (int ty = tm_render_range.top; ty < tm_render_range.bottom; ty++) {
        for (int tx = tm_render_range.left; tx < tm_render_range.right; tx++) {
            if (IsTileActive(tm_activity, (zfw_s_vec_2d_i){tx, ty})) {
                continue;
            }

            const zfw_s_vec_2d_i lp = {tx - tm_render_range.left, ty - tm_render_range.top};
            SetLightLevel(&lightmap, lp, LIGHT_LEVEL_LIMIT);
        }
    }

    if (!PropagateLights(&lightmap, temp_mem_arena)) {
        return (s_lightmap){0};
    }

    return lightmap;
}

bool RenderWorld(const zfw_s_rendering_context* const rendering_context, const s_world* const world, const zfw_s_textures* const textures, zfw_s_mem_arena* const temp_mem_arena) {
    ZFW_ZERO_OUT(rendering_context->state->view_mat);
    InitCameraViewMatrix(&rendering_context->state->view_mat, &world->cam, rendering_context->display_size);

    const zfw_s_rect_edges_i tilemap_render_range = TilemapRenderRange(&world->cam, rendering_context->display_size);

    RenderTilemap(rendering_context, &world->core.tilemap_core, &world->tilemap_tile_lifes, tilemap_render_range, textures);

    if (!world->player.killed) {
        RenderPlayer(rendering_context, world, textures);
    }

    RenderNPCs(rendering_context, &world->npcs, textures);

    RenderItemDrops(rendering_context, world->item_drops, world->item_drop_active_cnt, textures);

    RenderProjectiles(rendering_context, world->projectiles, world->proj_cnt, textures);

    RenderParticles(rendering_context, &world->particles, textures);

    const s_lightmap world_lightmap = GenWorldLightmap(temp_mem_arena, &world->core.tilemap_core.activity, tilemap_render_range, temp_mem_arena);

    if (!IsLightmapInitted(&world_lightmap)) {
        return false;
    }

    RenderLightmap(rendering_context, &world_lightmap, (zfw_s_vec_2d){tilemap_render_range.left * TILE_SIZE, tilemap_render_range.top * TILE_SIZE}, TILE_SIZE);

    ZFWFlush(rendering_context);

    return true;
}

bool LoadWorldCoreFromFile(s_world_core* const world_core, const t_world_filename* const filename) {
    assert(world_core && ZFW_IS_ZERO(*world_core));
    assert(filename);

    FILE* const fs = fopen((const char*)filename, "rb");

    if (!fs) {
        return false;
    }

    if (fread(world_core, sizeof(*world_core), 1, fs) == 0) {
        fclose(fs);
        return false;
    }

    fclose(fs);

    return true;
}

bool WriteWorldCoreToFile(const s_world_core* const world_core, const t_world_filename* const filename) {
    assert(world_core);
    assert(filename);

    FILE* const fs = fopen((const char*)filename, "wb");

    if (!fs) {
        fprintf(stderr, "Failed to open \"%s\"!\n", (const char*)filename);
        return false;
    }

    if (fwrite(world_core, sizeof(*world_core), 1, fs) == 0) {
        fprintf(stderr, "Failed to write to world file \"%s\"!\n", (const char*)filename);
        fclose(fs);
        return false;
    }

    fclose(fs);

    return true;
}

bool PlaceWorldTile(s_world* const world, const zfw_s_vec_2d_i pos, const e_tile_type type) {
    AddTile(&world->core.tilemap_core, pos, type);
    world->tilemap_tile_lifes[pos.y][pos.x] = 0;
    return true;
}

bool HurtWorldTile(s_world* const world, const zfw_s_vec_2d_i pos) {
    assert(IsTilePosInBounds(pos));
    assert(IsTileActive(&world->core.tilemap_core.activity, pos));

    world->tilemap_tile_lifes[pos.y][pos.x]++;

    const s_tile_type* const tile_type = &g_tile_types[world->core.tilemap_core.tile_types[pos.y][pos.x]];

    {
        // Spawn particles.
        const zfw_s_vec_2d tile_mid = {
            (pos.x + 0.5f) * TILE_SIZE,
            (pos.y + 0.5f) * TILE_SIZE
        };

        const int part_cnt = ZFWRandRangeI(3, 5);

        for (int i = 0; i < part_cnt; i++) {
            const zfw_s_vec_2d vel = {
                ZFWRandRangeIncl(-0.5f, 0.5f),
                ZFWRandRangeIncl(-2.5f, -1.0f)
            };

            SpawnParticleFromTemplate(&world->particles, tile_type->particle_template, tile_mid, vel, ZFWRandRot());
        }
    }

    if (world->tilemap_tile_lifes[pos.y][pos.x] == tile_type->life) {
        if (!DestroyWorldTile(world, pos)) {
            return false;
        }
    }

    return true;
}

bool DestroyWorldTile(s_world* const world, const zfw_s_vec_2d_i pos) {
assert(world);
    assert(IsTilePosInBounds(pos));
    assert(IsTileActive(&world->core.tilemap_core.activity, pos));

    RemoveTile(&world->core.tilemap_core, pos);

    // Spawn item drop.
    const zfw_s_vec_2d drop_pos = {(pos.x + 0.5f) * TILE_SIZE, (pos.y + 0.5f) * TILE_SIZE};
    const s_tile_type* const tile_type = &g_tile_types[world->core.tilemap_core.tile_types[pos.y][pos.x]];

    if (!SpawnItemDrop(world, drop_pos, tile_type->drop_item, 1)) {
        return false;
    }

    return true;
}

bool IsTilePosFree(const s_world* const world, const zfw_s_vec_2d_i tile_pos) {
    assert(world);
    assert(IsTilePosInBounds(tile_pos));
    assert(!IsTileActive(&world->core.tilemap_core.activity, tile_pos));

    const zfw_s_rect tile_collider = {
        tile_pos.x * TILE_SIZE,
        tile_pos.y * TILE_SIZE,
        TILE_SIZE,
        TILE_SIZE
    };

    // Check for player.
    const zfw_s_rect player_collider = PlayerCollider(world->player.pos);

    if (ZFWDoRectsInters(tile_collider, player_collider)) {
        return false;
    }

    // Check for NPCs.
    for (int i = 0; i < NPC_LIMIT; i++) {
        if (!IsNPCActive(&world->npcs.activity, i)) {
            continue;
        }

        const s_npc* const npc = &world->npcs.buf[i];
        const zfw_s_rect npc_collider = NPCCollider(npc->pos, npc->type);

        if (ZFWDoRectsInters(tile_collider, npc_collider)) {
            return false;
        }
    }

    // Check for projectiles.
    for (int i = 0; i < world->proj_cnt; i++) {
        const s_projectile* const proj = &world->projectiles[i];
        const zfw_s_rect proj_collider = ProjectileCollider(proj->type, proj->pos);

        if (ZFWDoRectsInters(tile_collider, proj_collider)) {
            return false;
        }
    }

    return true;
}

s_popup_text* SpawnPopupText(s_world* const world, const zfw_s_vec_2d pos, const float vel_y) {
    for (int i = 0; i < POPUP_TEXT_LIMIT; i++) {
        s_popup_text* const popup = &world->popup_texts[i];

        if (popup->alpha > POPUP_TEXT_INACTIVITY_ALPHA_THRESH) {
            continue;
        }

        ZFW_ZERO_OUT(*popup);

        popup->pos = pos;
        popup->alpha = 1.0f;
        popup->vel_y = vel_y;

        return popup;
    }

    fprintf(stderr, "Failed to spawn popup text due to insufficient space!\n");

    return NULL;
}
