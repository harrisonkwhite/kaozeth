#include <stdio.h>
#include <zfw_random.h>
#include "game.h"

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

bool InitWorld(s_world* const world, const t_world_filename* const filename) {
    assert(world && IS_ZERO(*world));
    assert(filename);

    if (!LoadWorldCoreFromFile(&world->core, filename)) {
        return false;
    }

    InitPlayer(&world->player, world->core.player_hp_max, &world->core.tilemap_core.activity);

    AddToInventory(world->player_inv_slots, PLAYER_INVENTORY_LENGTH, ek_item_type_copper_pickaxe, 1);
    AddToInventory(world->player_inv_slots, PLAYER_INVENTORY_LENGTH, ek_item_type_wooden_bow, 1);

    return true;
}

bool WorldTick(s_world* const world, const s_input_state* const input_state, const s_input_state* const input_state_last, const s_vec_2d_i display_size) {
    assert(display_size.x > 0 && display_size.y > 0); 

    ZERO_OUT(world->cursor_hover_str); // Reset this, for it can be overwritten over the course of this tick.

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

        world->player_inv_open = false;
    }

    world->cam_pos = world->player.pos;

    if (!ProcEnemySpawning(world)) {
        return false;
    }

    UpdateNPCs(world);
    ProcNPCDeaths(world); // NOTE: Might need to defer this until later in the tick.

    UpdateItemDrops(world);

    UpdatePlayerInventoryHotbarSlotSelected(&world->player_inv_hotbar_slot_selected, input_state, input_state_last);

    if (!world->player.killed) {
        if (IsKeyPressed(ek_key_code_escape, input_state, input_state_last)) {
            world->player_inv_open = !world->player_inv_open;
        }
    }

    if (world->player_inv_open) {
        ProcPlayerInventoryOpenState(world, input_state, input_state_last, display_size);
    }

    if (!ProcItemUsage(world, input_state, display_size)) {
        return false;
    }

    if (!UpdateProjectiles(world)) {
        return false;
    }

    //
    // Popup Texts
    //
    for (int i = 0; i < POPUP_TEXT_LIMIT; i++) {
        s_popup_text* const popup = &world->popup_texts[i];

        assert(IsNullTerminated(popup->str, POPUP_TEXT_STR_BUF_SIZE));
        assert(popup->alpha >= 0.0f && popup->alpha <= 1.0f);

        if (popup->alpha <= POPUP_TEXT_INACTIVITY_ALPHA_THRESH) {
            continue;
        }

        if (fabs(popup->vel_y) <= POPUP_TEXT_FADE_VEL_Y_ABS_THRESH) {
            popup->alpha *= POPUP_TEXT_ALPHA_MULT;
        }

        popup->pos.y += popup->vel_y;

        popup->vel_y *= POPUP_TEXT_VEL_Y_MULT;
    }

    //
    // NPC Hovering
    //

    // TODO: Clean up messy state system. No need to check for the below if we're in the inventory - unless we want to perfectly model Terraria.

    const s_vec_2d cursor_cam_pos = DisplayToCameraPos(input_state->mouse_pos, world->cam_pos, display_size);

    for (int i = 0; i < NPC_LIMIT; i++) {
        if (!IsNPCActive(&world->npcs.activity, i)) {
            continue;
        }

        const s_npc* const npc = &world->npcs.buf[i];
        const s_npc_type* const npc_type = &g_npc_types[npc->type];
        const s_rect npc_collider = NPCCollider(npc->pos, npc->type);

        if (IsPointInRect(cursor_cam_pos, npc_collider)) {
            snprintf(world->cursor_hover_str, sizeof(world->cursor_hover_str), "%s (%d/%d)", npc_type->name, npc->hp, npc_type->hp_max);
        }
    }

    return true;
}

void RenderWorld(const s_rendering_context* const rendering_context, const s_world* const world, const s_textures* const textures) {
    ZERO_OUT(rendering_context->state->view_mat);
    InitCameraViewMatrix(&rendering_context->state->view_mat, world->cam_pos, rendering_context->display_size);

    {
        const s_vec_2d cam_tl = CameraTopLeft(world->cam_pos, rendering_context->display_size);
        const s_vec_2d cam_size = CameraSize(rendering_context->display_size);

        s_rect_edges_i tilemap_render_range = {
            .left = floorf(cam_tl.x / TILE_SIZE),
            .top = floorf(cam_tl.y / TILE_SIZE),
            .right = ceilf((cam_tl.x + cam_size.x) / TILE_SIZE),
            .bottom = ceilf((cam_tl.y + cam_size.y) / TILE_SIZE)
        };

        // Clamp the tilemap render range within tilemap bounds.
        tilemap_render_range.left = CLAMP(tilemap_render_range.left, 0, TILEMAP_WIDTH - 1);
        tilemap_render_range.top = CLAMP(tilemap_render_range.top, 0, TILEMAP_HEIGHT - 1);
        tilemap_render_range.right = CLAMP(tilemap_render_range.right, 0, TILEMAP_WIDTH);
        tilemap_render_range.bottom = CLAMP(tilemap_render_range.bottom, 0, TILEMAP_HEIGHT);

        RenderTilemap(rendering_context, &world->core.tilemap_core, &world->tilemap_tile_lifes, tilemap_render_range, textures);
    }

    if (!world->player.killed) {
        RenderPlayer(rendering_context, world, textures);
    }

    RenderNPCs(rendering_context, &world->npcs, textures);

    RenderProjectiles(rendering_context, world->projectiles, world->proj_cnt, textures);

    RenderItemDrops(rendering_context, world->item_drops, world->item_drop_active_cnt, textures);

    Flush(rendering_context);
}

bool RenderWorldUI(const s_rendering_context* const rendering_context, const s_world* const world, const s_vec_2d cursor_pos, const s_textures* const textures, const s_fonts* const fonts, s_mem_arena* const temp_mem_arena) {
    const s_vec_2d_i ui_size = UISize(rendering_context->display_size);
    const s_vec_2d cursor_ui_pos = DisplayToUIPos(cursor_pos);

    //
    // Tile Highlight
    //
    {
        const s_vec_2d cursor_cam_pos = DisplayToCameraPos(cursor_pos, world->cam_pos, rendering_context->display_size);
        const s_vec_2d_i cursor_tile_pos = CameraToTilePos(cursor_cam_pos);
        const s_vec_2d cursor_cam_pos_snapped_to_tilemap = {cursor_tile_pos.x * TILE_SIZE, cursor_tile_pos.y * TILE_SIZE};

        const s_vec_2d highlight_pos = CameraToUIPos(cursor_cam_pos_snapped_to_tilemap, world->cam_pos, rendering_context->display_size);
        const float highlight_size = (float)(TILE_SIZE / CAMERA_SCALE) * TILE_SIZE;
        const s_rect highlight_rect = {
            .x = highlight_pos.x,
            .y = highlight_pos.y,
            .width = highlight_size,
            .height = highlight_size
        };
        RenderRect(rendering_context, highlight_rect, (s_color){1.0f, 1.0f, 1.0f, TILE_HIGHLIGHT_ALPHA});
    }

    //
    // Popup Texts
    //
    for (int i = 0; i < POPUP_TEXT_LIMIT; i++) {
        const s_popup_text* const popup = &world->popup_texts[i];

        if (popup->alpha <= POPUP_TEXT_INACTIVITY_ALPHA_THRESH) {
            continue;
        }

        const s_vec_2d popup_display_pos = CameraToDisplayPos(popup->pos, world->cam_pos, rendering_context->display_size);
        const s_vec_2d popup_ui_pos = DisplayToUIPos(popup_display_pos);
        const s_color popup_blend = {1.0f, 1.0f, 1.0f, popup->alpha};

        assert(popup->str[0] != '\0' && "Popup text string cannot be empty!\n");

        if (!RenderStr(rendering_context, popup->str, ek_font_eb_garamond_32, fonts, popup_ui_pos, ek_str_hor_align_center, ek_str_ver_align_center, popup_blend, temp_mem_arena)) {
            return false;
        }
    }

    //
    // Death Text
    //
    if (world->player.killed) {
        if (!RenderStr(rendering_context, DEATH_TEXT, ek_font_eb_garamond_48, fonts, (s_vec_2d){ui_size.x / 2.0f, ui_size.y / 2.0f}, ek_str_hor_align_center, ek_str_ver_align_center, WHITE, temp_mem_arena)) {
            return false;
        }
    }

    //
    // Player Health
    //
    {
        const s_vec_2d hp_text_pos = {
            ui_size.x / 2.0f,
            64.0f
        };

        char hp_str[8] = {0};
        snprintf(hp_str, sizeof(hp_str), "%d/%d", world->player.hp, world->core.player_hp_max);

        if (!RenderStr(rendering_context, hp_str, ek_font_eb_garamond_32, fonts, hp_text_pos, ek_str_hor_align_center, ek_str_ver_align_center, WHITE, temp_mem_arena)) {
            return false;
        }
    }

    //
    // Player Inventory
    //

    // Draw a backdrop if the player inventory is open.
    if (world->player_inv_open) {
        const s_vec_2d_i ui_size = UISize(rendering_context->display_size);
        const s_rect bg_rect = {0.0f, 0.0f, ui_size.x, ui_size.y};
        RenderRect(rendering_context, bg_rect, (s_color){0.0f, 0.0f, 0.0f, PLAYER_INVENTORY_BG_ALPHA});
    }

    // Get positions of all slots.
    s_vec_2d player_inv_slot_positions[PLAYER_INVENTORY_LENGTH];
    LoadPlayerInventorySlotPositions(&player_inv_slot_positions, ui_size);

    // Render the hotbar.
    for (int i = 0; i < PLAYER_INVENTORY_COLUMN_CNT; i++) {
        const s_inventory_slot* const slot = &world->player_inv_slots[i];

        const s_color slot_color = i == world->player_inv_hotbar_slot_selected ? YELLOW : WHITE;

        if (!RenderInventorySlot(rendering_context, *slot, player_inv_slot_positions[i], slot_color, textures, fonts, temp_mem_arena)) {
            return false;
        }
    }

    // Render the body if open.
    if (world->player_inv_open) {
        for (int i = PLAYER_INVENTORY_COLUMN_CNT; i < PLAYER_INVENTORY_LENGTH; i++) {
            const s_inventory_slot* const slot = &world->player_inv_slots[i];

            if (!RenderInventorySlot(rendering_context, *slot, player_inv_slot_positions[i], WHITE, textures, fonts, temp_mem_arena)) {
                return false;
            }
        }
    }

    //
    // Cursor Hover String and Item
    //
    if (world->cursor_hover_str[0]) {
        if (!RenderStr(rendering_context, world->cursor_hover_str, ek_font_eb_garamond_24, fonts, cursor_ui_pos, ek_str_hor_align_left, ek_str_ver_align_top, WHITE, temp_mem_arena)) {
            return false;
        }
    }

    if (world->cursor_item_held_quantity > 0) {
        RenderSprite(rendering_context, g_item_types[world->cursor_item_held_type].icon_spr, textures, cursor_ui_pos, (s_vec_2d){0.5f, 0.5f}, (s_vec_2d){CAMERA_SCALE / UI_SCALE, CAMERA_SCALE / UI_SCALE}, 0.0f, WHITE);
    }

    return true;
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
