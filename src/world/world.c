#include <stdio.h>
#include "world.h"
#include "zfw_math.h"
#include "zfw_random.h"
#include "zfw_utils.h"

#define RESPAWN_TIME 120

static void InitCameraViewMatrix(t_matrix_4x4* const mat, const s_vec_2d cam_pos, const s_vec_2d_i display_size) {
    assert(mat && IS_ZERO(*mat));
    assert(display_size.x > 0 && display_size.y > 0);

    const s_vec_2d view_pos = {
        (-cam_pos.x * CAMERA_SCALE) + (display_size.x / 2.0f),
        (-cam_pos.y * CAMERA_SCALE) + (display_size.y / 2.0f)
    };

    InitIdenMatrix4x4(mat);
    TranslateMatrix4x4(mat, view_pos);
    ScaleMatrix4x4(mat, CAMERA_SCALE);
}

bool InitWorld(s_world* const world, const t_world_filename* const filename, s_mem_arena* const temp_mem_arena) {
    assert(world && IS_ZERO(*world));
    assert(filename);

    if (!InitMemArena(&world->mem_arena, WORLD_MEM_ARENA_SIZE)) {
        fprintf(stderr, "Failed to initialise world memory arena!\n");
        return false;
    }

    if (!LoadWorldCoreFromFile(&world->core, filename)) {
        return false;
    }

    InitPlayer(&world->player, world->core.player_hp_max, &world->core.tilemap_core.activity);

    world->cam_pos = world->player.pos;

    AddToInventory((s_inventory_slot*)world->player_inv_slots, PLAYER_INVENTORY_LEN, ek_item_type_copper_pickaxe, 1);

#if 0
    world->lightmap = GenLightmap(&world->mem_arena, (s_vec_2d_i){TILEMAP_WIDTH, TILEMAP_HEIGHT});

    if (IS_ZERO(world->lightmap)) {
        return false;
    }

    if (!PropagateLights(&world->lightmap, (const t_byte*)world->core.tilemap_core.activity, temp_mem_arena, false)) {
        return false;
    }

    if (IS_ZERO(world->lightmap)) {
        fprintf(stderr, "Failed to generate world lightmap!");
        return false;
    }
#endif

    return true;
}

void CleanWorld(s_world* const world) {
    CleanMemArena(&world->mem_arena);
}

bool WorldTick(s_world* const world, const t_settings* const settings, const s_input_state* const input_state, const s_input_state* const input_state_last, const s_vec_2d_i display_size, s_audio_sys* const audio_sys, const s_sound_types* const snd_types, s_mem_arena* const temp_mem_arena) {
    assert(display_size.x > 0 && display_size.y > 0); 

    const s_vec_2d cam_size = CameraSize(display_size);

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

            ZERO_OUT(world->player);
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

    if (!ProcItemUsage(world, input_state, display_size, temp_mem_arena)) {
        return false;
    }

    if (!UpdateProjectiles(world)) {
        return false;
    }

    UpdateWorldUI(world, input_state, input_state_last, display_size);

    // Update the camera.
    {
        const s_vec_2d cam_pos_dest = world->player.pos;

        if (SettingToggle(settings, ek_setting_smooth_camera)) {
            world->cam_pos = LerpVec2D(world->cam_pos, cam_pos_dest, CAMERA_LERP);

            world->cam_pos = (s_vec_2d){
                CLAMP(world->cam_pos.x, cam_size.x / 2.0f, WORLD_WIDTH - (cam_size.x / 2.0f)),
                CLAMP(world->cam_pos.y, cam_size.y / 2.0f, WORLD_HEIGHT - (cam_size.y / 2.0f))
            };
        } else {
            world->cam_pos = cam_pos_dest;
        }
    }

    UpdateParticles(&world->particles, GRAVITY);

    return true;
}

static s_rect_edges_i TilemapRenderRange(const s_vec_2d cam_pos, const s_vec_2d_i display_size) {
    assert(display_size.x > 0 && display_size.y > 0);

    const s_vec_2d cam_tl = CameraTopLeft(cam_pos, display_size);
    const s_vec_2d cam_size = CameraSize(display_size);

    s_rect_edges_i render_range = {
        .left = floorf(cam_tl.x / TILE_SIZE),
        .top = floorf(cam_tl.y / TILE_SIZE),
        .right = ceilf((cam_tl.x + cam_size.x) / TILE_SIZE),
        .bottom = ceilf((cam_tl.y + cam_size.y) / TILE_SIZE)
    };

    // Clamp the tilemap render range within tilemap bounds.
    render_range.left = CLAMP(render_range.left, 0, TILEMAP_WIDTH - 1);
    render_range.top = CLAMP(render_range.top, 0, TILEMAP_HEIGHT - 1);
    render_range.right = CLAMP(render_range.right, 0, TILEMAP_WIDTH);
    render_range.bottom = CLAMP(render_range.bottom, 0, TILEMAP_HEIGHT);

    return render_range;
}

static s_lightmap GenWorldLightmap(s_mem_arena* const mem_arena, const t_tilemap_activity* const tm_activity, const s_rect_edges_i tm_render_range) {
    const s_lightmap lightmap = GenLightmap(mem_arena, (s_vec_2d_i){tm_render_range.right - tm_render_range.left, tm_render_range.bottom - tm_render_range.top});

    if (!IS_ZERO(lightmap)) {
        for (int ty = tm_render_range.top; ty < tm_render_range.bottom; ty++) {
            for (int tx = tm_render_range.left; tx < tm_render_range.right; tx++) {
                if (IsTileActive(tm_activity, (s_vec_2d_i){tx, ty})) {
                    continue;
                }

                PropagateLight(&lightmap, (s_vec_2d_i){tx - tm_render_range.left, ty - tm_render_range.top}, LIGHT_LEVEL_LIMIT);
            }
        }
    }

    return lightmap;
}

void RenderWorld(const s_rendering_context* const rendering_context, const s_world* const world, const s_textures* const textures, s_mem_arena* const temp_mem_arena) {
    ZERO_OUT(rendering_context->state->view_mat);
    InitCameraViewMatrix(&rendering_context->state->view_mat, world->cam_pos, rendering_context->display_size);

    const s_rect_edges_i tilemap_render_range = TilemapRenderRange(world->cam_pos, rendering_context->display_size);

    RenderTilemap(rendering_context, &world->core.tilemap_core, &world->tilemap_tile_lifes, tilemap_render_range, textures);

    if (!world->player.killed) {
        RenderPlayer(rendering_context, world, textures);
    }

    RenderNPCs(rendering_context, &world->npcs, textures);

    RenderItemDrops(rendering_context, world->item_drops, world->item_drop_active_cnt, textures);

    RenderProjectiles(rendering_context, world->projectiles, world->proj_cnt, textures);

    RenderParticles(rendering_context, &world->particles, textures);

    const s_lightmap world_lightmap = GenWorldLightmap(temp_mem_arena, &world->core.tilemap_core.activity, tilemap_render_range);

    if (IS_ZERO(world_lightmap)) {
        // TODO: Return false!
        return;
    }

    RenderLightmap(rendering_context, &world_lightmap, (s_vec_2d){tilemap_render_range.left * TILE_SIZE, tilemap_render_range.top * TILE_SIZE}, TILE_SIZE);

    Flush(rendering_context);
}

bool LoadWorldCoreFromFile(s_world_core* const world_core, const t_world_filename* const filename) {
    assert(world_core && IS_ZERO(*world_core));
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

bool PlaceWorldTile(s_world* const world, const s_vec_2d_i pos, const e_tile_type type, s_mem_arena* const temp_mem_arena) {
    AddTile(&world->core.tilemap_core, pos, type);
    world->tilemap_tile_lifes[pos.y][pos.x] = 0;

#if 0
    // Update lightmap.
    {
        t_byte* const seed = MEM_ARENA_PUSH_TYPE_MANY(temp_mem_arena, t_byte, BITS_TO_BYTES(world->lightmap.size.x * world->lightmap.size.y));

        if (!seed) {
            return false;
        }

        memset(seed, 255, BITS_TO_BYTES(world->lightmap.size.x * world->lightmap.size.y));

        world->lightmap.buf[IndexFrom2D(pos, world->lightmap.size.x)] = 0;

        const s_vec_2d_i neighbour_pos_offsets[] = {
            {1, 0},
            {-1, 0},
            {0, 1},
            {0, -1}
        }; // TODO: Move this to lighting.h.

        for (int i = 0; i < STATIC_ARRAY_LEN(neighbour_pos_offsets); i++) {
            const s_vec_2d_i neighbour_pos = Vec2DISum(pos, neighbour_pos_offsets[i]);

            DeactivateBit(IndexFrom2D(neighbour_pos, world->lightmap.size.x), seed, world->lightmap.size.x * world->lightmap.size.y);
        }

        if (!PropagateLights(&world->lightmap, seed, temp_mem_arena, true)) {
            return false;
        }
    }
#endif

    return true;
}

bool HurtWorldTile(s_world* const world, const s_vec_2d_i pos, s_mem_arena* const temp_mem_arena) {
    assert(IsTilePosInBounds(pos));
    assert(IsTileActive(&world->core.tilemap_core.activity, pos));

    world->tilemap_tile_lifes[pos.y][pos.x]++;

    const s_tile_type* const tile_type = &g_tile_types[world->core.tilemap_core.tile_types[pos.y][pos.x]];

    {
        // Spawn particles.
        const s_vec_2d tile_mid = {
            (pos.x + 0.5f) * TILE_SIZE,
            (pos.y + 0.5f) * TILE_SIZE
        };

        const int part_cnt = RandRangeI(3, 5);

        for (int i = 0; i < part_cnt; i++) {
            const s_vec_2d vel = {
                RandRangeIncl(-0.5f, 0.5f),
                RandRangeIncl(-2.5f, -1.0f)
            };

            SpawnParticleFromTemplate(&world->particles, tile_type->particle_template, tile_mid, vel, RandRot());
        }
    }

    if (world->tilemap_tile_lifes[pos.y][pos.x] == tile_type->life) {
        if (!DestroyWorldTile(world, pos, temp_mem_arena)) {
            return false;
        }
    }

    return true;
}

bool DestroyWorldTile(s_world* const world, const s_vec_2d_i pos, s_mem_arena* const temp_mem_arena) {
assert(world);
    assert(IsTilePosInBounds(pos));
    assert(IsTileActive(&world->core.tilemap_core.activity, pos));

    RemoveTile(&world->core.tilemap_core, pos);

#if 0
    // Update lightmap.
    t_byte* const seed = MEM_ARENA_PUSH_TYPE_MANY(temp_mem_arena, t_byte, BITS_TO_BYTES(world->lightmap.size.x * world->lightmap.size.y));

    if (!seed) {
        return false;
    }

    memset(seed, 255, BITS_TO_BYTES(world->lightmap.size.x * world->lightmap.size.y));

    DeactivateBit(IndexFrom2D(pos, world->lightmap.size.x), seed, world->lightmap.size.x * world->lightmap.size.y);

    if (!PropagateLights(&world->lightmap, seed, temp_mem_arena, false)) {
        return false;
    }
#endif

    // Spawn item drop.
    const s_vec_2d drop_pos = {(pos.x + 0.5f) * TILE_SIZE, (pos.y + 0.5f) * TILE_SIZE};
    const s_tile_type* const tile_type = &g_tile_types[world->core.tilemap_core.tile_types[pos.y][pos.x]];

    if (!SpawnItemDrop(world, drop_pos, tile_type->drop_item, 1)) {
        return false;
    }

    return true;
}

bool IsTilePosFree(const s_world* const world, const s_vec_2d_i tile_pos) {
    assert(world);
    assert(IsTilePosInBounds(tile_pos));
    assert(!IsTileActive(&world->core.tilemap_core.activity, tile_pos));

    const s_rect tile_collider = {
        tile_pos.x * TILE_SIZE,
        tile_pos.y * TILE_SIZE,
        TILE_SIZE,
        TILE_SIZE
    };

    // Check for player.
    const s_rect player_collider = PlayerCollider(world->player.pos);

    if (DoRectsInters(tile_collider, player_collider)) {
        return false;
    }

    // Check for NPCs.
    for (int i = 0; i < NPC_LIMIT; i++) {
        if (!IsNPCActive(&world->npcs.activity, i)) {
            continue;
        }

        const s_npc* const npc = &world->npcs.buf[i];
        const s_rect npc_collider = NPCCollider(npc->pos, npc->type);

        if (DoRectsInters(tile_collider, npc_collider)) {
            return false;
        }
    }

    // Check for projectiles.
    for (int i = 0; i < world->proj_cnt; i++) {
        const s_projectile* const proj = &world->projectiles[i];
        const s_rect proj_collider = ProjectileCollider(proj->type, proj->pos);

        if (DoRectsInters(tile_collider, proj_collider)) {
            return false;
        }
    }

    return true;
}

s_popup_text* SpawnPopupText(s_world* const world, const s_vec_2d pos, const float vel_y) {
    for (int i = 0; i < POPUP_TEXT_LIMIT; i++) {
        s_popup_text* const popup = &world->popup_texts[i];

        if (popup->alpha > POPUP_TEXT_INACTIVITY_ALPHA_THRESH) {
            continue;
        }

        ZERO_OUT(*popup);

        popup->pos = pos;
        popup->alpha = 1.0f;
        popup->vel_y = vel_y;

        return popup;
    }

    fprintf(stderr, "Failed to spawn popup text due to insufficient space!\n");

    return NULL;
}
