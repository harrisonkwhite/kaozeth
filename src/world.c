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

static bool LoadWorldCoreFromFile(s_world_core* const world_core, const char* const filename) {
    assert(world_core && IS_ZERO(*world_core));
    assert(filename);

    FILE* const fs = fopen(filename, "rb");

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

static bool WriteWorldCoreToFile(const s_world_core* const world_core, const char* const filename) {
    assert(world_core);
    assert(filename);

    FILE* const fs = fopen(filename, "wb");

    if (!fs) {
        fprintf(stderr, "Failed to open \"%s\"!\n", filename);
        return false;
    }

    if (fwrite(world_core, sizeof(*world_core), 1, fs) == 0) {
        fprintf(stderr, "Failed to write to world file \"%s\"!\n", filename);
        fclose(fs);
        return false;
    }

    fclose(fs);

    return true;
}

bool GenWorld(const char* const filename) {
    s_world_core world_core = {0};

    world_core.player_hp_max = PLAYER_INIT_HP_MAX;

    // Generate the tilemap.
    int ty = TILEMAP_HEIGHT / 3;

    for (; ty < TILEMAP_HEIGHT / 2; ty++) {
        for (int tx = 0; tx < TILEMAP_WIDTH; tx++) {
            PlaceTile(&world_core.tilemap, (s_vec_2d_i){tx, ty}, ek_tile_type_dirt);
        }
    }

    for (; ty < TILEMAP_HEIGHT; ty++) {
        for (int tx = 0; tx < TILEMAP_WIDTH; tx++) {
            PlaceTile(&world_core.tilemap, (s_vec_2d_i){tx, ty}, ek_tile_type_stone);
        }
    }

    // Write the world core to a file.
    return WriteWorldCoreToFile(&world_core, filename);
}

bool InitWorld(s_world* const world, const char* const filename) {
    assert(world && IS_ZERO(*world));
    assert(filename);

    if (!LoadWorldCoreFromFile(&world->core, filename)) {
        return false;
    }

    InitPlayer(&world->player, world->core.player_hp_max);

    AddToInventory(world->player_inv_slots, PLAYER_INVENTORY_LENGTH, ek_item_type_copper_pickaxe, 1);
    AddToInventory(world->player_inv_slots, PLAYER_INVENTORY_LENGTH, ek_item_type_wooden_bow, 1);

    return true;
}

bool WorldTick(s_world* const world, const s_input_state* const input_state, const s_input_state* const input_state_last, const s_vec_2d_i display_size) {
    assert(world);
    assert(input_state);
    assert(input_state_last);
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
            ZERO_OUT(world->player);
            InitPlayer(&world->player, world->core.player_hp_max);
        }
    }

    //
    // Camera
    //
    world->cam_pos = world->player.pos;

    //
    // NPC Spawning
    //
    if (!ProcEnemySpawning(world)) {
        return false;
    }

    //
    // NPCs
    //
    RunNPCTicks(world);
    ProcNPCDeaths(world); // NOTE: Might need to defer this until later in the tick.

    //
    // Item Drops
    //
    UpdateItemDrops(world);

    //
    // Player Inventory
    //
    if (IsKeyPressed(ek_key_code_escape, input_state, input_state_last)) {
        world->player_inv_open = !world->player_inv_open;
    }

    for (int i = 0; i < PLAYER_INVENTORY_COLUMN_CNT; i++) {
        if (IsKeyPressed(ek_key_code_1 + i, input_state, input_state_last)) {
            world->player_inv_hotbar_slot_selected = i;
            break;
        }
    }

    switch (input_state->mouse_scroll) {
        case ek_mouse_scroll_state_down:
            world->player_inv_hotbar_slot_selected++;
            world->player_inv_hotbar_slot_selected %= PLAYER_INVENTORY_COLUMN_CNT;
            break;

        case ek_mouse_scroll_state_up:
            world->player_inv_hotbar_slot_selected--;

            if (world->player_inv_hotbar_slot_selected < 0) {
                world->player_inv_hotbar_slot_selected += PLAYER_INVENTORY_COLUMN_CNT;
            }

            break;

        default: break;
    }

    if (world->player_inv_open) {
        const s_vec_2d_i ui_size = UISize(display_size);
        const s_vec_2d cursor_ui_pos = DisplayToUIPos(input_state->mouse_pos);

        s_vec_2d inv_slot_positions[PLAYER_INVENTORY_LENGTH];
        LoadPlayerInventorySlotPositions(&inv_slot_positions, ui_size);

        for (int i = 0; i < PLAYER_INVENTORY_LENGTH; i++) {
            s_inventory_slot* const slot = &world->player_inv_slots[i];

            const s_rect slot_collider = {
                inv_slot_positions[i].x - (INVENTORY_SLOT_SIZE / 2.0f),
                inv_slot_positions[i].y - (INVENTORY_SLOT_SIZE / 2.0f),
                INVENTORY_SLOT_SIZE,
                INVENTORY_SLOT_SIZE
            };

            if (IsPointInRect(cursor_ui_pos, slot_collider)) {
                const bool clicked = IsMouseButtonPressed(ek_mouse_button_code_left, input_state, input_state_last);

                if (slot->quantity > 0) {
                    if (slot->quantity == 1) {
                        snprintf(world->cursor_hover_str, sizeof(world->cursor_hover_str), "%s", g_item_types[slot->item_type].name);
                    } else {
                        snprintf(world->cursor_hover_str, sizeof(world->cursor_hover_str), "%s (%d)", g_item_types[slot->item_type].name, slot->quantity);
                    }

                    if (clicked) {
                        world->cursor_item_held_type = slot->item_type;
                        world->cursor_item_held_quantity = slot->quantity;

                        slot->quantity = 0;
                    }
                } else {
                    if (clicked && world->cursor_item_held_quantity > 0) {
                        slot->item_type = world->cursor_item_held_type;
                        slot->quantity = world->cursor_item_held_quantity;

                        world->cursor_item_held_quantity = 0;
                    }
                }

                break;
            }
        }
    }

    assert(world->player_inv_hotbar_slot_selected >= 0 && world->player_inv_hotbar_slot_selected < PLAYER_INVENTORY_COLUMN_CNT);

    //
    // Item Usage
    //
    if (!world->player_inv_open) {
        s_inventory_slot* const cur_slot = &world->player_inv_slots[world->player_inv_hotbar_slot_selected];

        if (cur_slot->quantity > 0) {
            const s_item_type* const active_item = &g_item_types[cur_slot->item_type];

            if (IsMouseButtonPressed(ek_mouse_button_code_left, input_state, input_state_last)) {
                const s_vec_2d mouse_cam_pos = DisplayToCameraPos(input_state->mouse_pos, world->cam_pos, display_size);

                const s_vec_2d_i mouse_tile_pos = {
                    floorf(mouse_cam_pos.x / TILE_SIZE),
                    floorf(mouse_cam_pos.y / TILE_SIZE)
                };

                // Perform unique action based on the item use type.
                bool used = false; // Did we use the item?

                switch (active_item->use_type) {
                    case ek_item_use_type_tile_place:
                        if (IsTilePosInBounds(mouse_tile_pos) && !IsTileActive(&world->core.tilemap.activity, mouse_tile_pos)) {
                            PlaceTile(&world->core.tilemap, mouse_tile_pos, active_item->tile_place_type);
                            used = true;
                        }

                        break;

                    case ek_item_use_type_tile_destroy:
                        if (IsTilePosInBounds(mouse_tile_pos) && IsTileActive(&world->core.tilemap.activity, mouse_tile_pos)) {
                            DestroyTile(world, mouse_tile_pos);
                            used = true;
                        }

                        break;

                    case ek_item_use_type_shoot:
                        {
                            const s_vec_2d dir = Vec2DDir(world->player.pos, mouse_cam_pos);
                            const s_vec_2d vel = Vec2DScaled(dir, active_item->shoot_proj_spd);

                            if (!SpawnProjectile(world, active_item->shoot_proj_type, true, active_item->shoot_proj_dmg, world->player.pos, vel)) {
                                return false;
                            }

                            used = true;
                        }

                        break;
               }

                // Handle consuming the item.
                if (used && active_item->consume_on_use) {
                    cur_slot->quantity--;
                }
            }
        }
    }

    //
    // Projectiles
    //
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

        RenderTilemap(rendering_context, &world->core.tilemap, tilemap_render_range, textures);
    }

    if (!world->player.killed) {
        RenderPlayer(rendering_context, world, textures);
    }

    RenderNPCs(rendering_context, &world->npcs, textures);

    RenderProjectiles(rendering_context, world->projectiles, world->proj_cnt, textures);

    // Render item drops.
    for (int i = 0; i < world->item_drop_active_cnt; i++) {
        const s_item_drop* const drop = &world->item_drops[i];

        const e_sprite spr = g_item_types[drop->item_type].spr;

        RenderSprite(rendering_context, spr, textures, drop->pos, (s_vec_2d){0.5f, 0.5f}, (s_vec_2d){1.0f, 1.0f}, 0.0f, WHITE);
    }

    Flush(rendering_context);
}

bool RenderWorldUI(const s_rendering_context* const rendering_context, const s_world* const world, const s_vec_2d cursor_ui_pos, const s_textures* const textures, const s_fonts* const fonts, s_mem_arena* const temp_mem_arena) {
    const s_vec_2d_i ui_size = UISize(rendering_context->display_size);

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
        RenderSprite(rendering_context, g_item_types[world->cursor_item_held_type].spr, textures, cursor_ui_pos, (s_vec_2d){0.5f, 0.5f}, (s_vec_2d){1.0f, 1.0f}, 0.0f, WHITE);
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
