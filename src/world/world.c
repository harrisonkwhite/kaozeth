#include <stdio.h>
#include "world.h"

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

    world->lightmap = GenLightmap(&world->mem_arena, (s_vec_2d_i){TILEMAP_WIDTH, TILEMAP_HEIGHT}, (const t_byte*)world->core.tilemap_core.activity, temp_mem_arena);

    if (IS_ZERO(world->lightmap)) {
        fprintf(stderr, "Failed to generate world lightmap!");
        return false;
    }

    return true;
}

void CleanWorld(s_world* const world) {
    CleanMemArena(&world->mem_arena);
}

bool WorldTick(s_world* const world, const s_input_state* const input_state, const s_input_state* const input_state_last, const s_vec_2d_i display_size, s_audio_sys* const audio_sys, const s_sound_types* const snd_types) {
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

    if (!UpdateItemDrops(world, audio_sys, snd_types)) {
        return false;
    }

    if (!ProcItemUsage(world, input_state, display_size)) {
        return false;
    }

    if (!UpdateProjectiles(world)) {
        return false;
    }

    UpdateWorldUI(world, input_state, input_state_last, display_size);

    //
    // Camera
    //
    {
        const s_vec_2d cam_pos_dest = world->player.pos;

        world->cam_pos = LerpVec2D(world->cam_pos, cam_pos_dest, CAMERA_LERP);

        world->cam_pos = (s_vec_2d){
            CLAMP(world->cam_pos.x, cam_size.x / 2.0f, WORLD_WIDTH - (cam_size.x / 2.0f)),
            CLAMP(world->cam_pos.y, cam_size.y / 2.0f, WORLD_HEIGHT - (cam_size.y / 2.0f))
        };
    }

    //
    //
    //
    //ZERO_OUT(world->mouse_hover_str);
    //LoadMouseHoverStr(&world->mouse_hover_str, input_state->mouse_pos, world, display_size);

    return true;
}

void RenderWorld(const s_rendering_context* const rendering_context, const s_world* const world, const s_textures* const textures) {
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

    RenderLightmap(rendering_context, &world->lightmap, tilemap_render_range, TILE_SIZE);

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
