#include <stdio.h>
#include "game.h"

static void InitCameraViewMatrix4x4(t_matrix_4x4* const mat, const s_vec_2d cam_pos, const s_vec_2d_i display_size) {
    assert(mat && IsZero(mat, sizeof(*mat)));
    assert(display_size.x > 0 && display_size.y > 0);

    const s_vec_2d view_pos = {
        (-cam_pos.x * CAMERA_SCALE) + (display_size.x / 2.0f),
        (-cam_pos.y * CAMERA_SCALE) + (display_size.y / 2.0f)
    };

    InitIdenMatrix4x4(mat);
    TranslateMatrix4x4(mat, view_pos);
    ScaleMatrix4x4(mat, CAMERA_SCALE);
}

void InitWorld(s_world* const world) {
    assert(world && IsZero(world, sizeof(*world)));

    world->player_pos.x = TILE_SIZE * TILEMAP_WIDTH * 0.5f;

    SpawnNPC(&world->npcs, (s_vec_2d){TILE_SIZE * TILEMAP_WIDTH * 0.25f, 0.0f}, ek_npc_type_slime);

    for (int ty = TILEMAP_HEIGHT / 2; ty < TILEMAP_HEIGHT; ty++) {
        for (int tx = 0; tx < TILEMAP_WIDTH; tx++) {
            ActivateTile(&world->tilemap_activity, (s_vec_2d_i){tx, ty});
        }
    }
}

#define PLAYER_ORIGIN (s_vec_2d){0.5f, 0.5f}

static s_rect PlayerCollider(const s_vec_2d pos) {
    return ColliderFromSprite(ek_sprite_player, pos, PLAYER_ORIGIN);
}

void WorldTick(s_world* const world, const s_input_state* const input_state, const s_input_state* const input_state_last, const s_vec_2d_i display_size) {
    assert(world);
    assert(input_state);
    assert(input_state_last);
    assert(display_size.x > 0 && display_size.y > 0);

    //
    // Player Movement
    //
    {
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

    //
    // Camera
    //
    world->cam_pos = world->player_pos;

    //
    // NPCs
    //
    RunNPCTicks(world);

    //
    // Tilemap Interaction
    //
    if (IsMouseButtonPressed(ek_mouse_button_code_left, input_state, input_state_last)) {
        const s_vec_2d mouse_cam_pos = DisplayToCameraPos(input_state->mouse_pos, world->cam_pos, display_size);

        const s_vec_2d_i mouse_tile_pos = {
            floorf(mouse_cam_pos.x / TILE_SIZE),
            floorf(mouse_cam_pos.y / TILE_SIZE)
        };

        if (IsTilePosInBounds(mouse_tile_pos)) {
            DeactivateTile(&world->tilemap_activity, mouse_tile_pos);
        }
    }

    //
    // Player Inventory
    //
    if (IsKeyPressed(ek_key_code_escape, input_state, input_state_last)) {
        world->player_inventory_open = !world->player_inventory_open;
    }

    for (int i = 0; i < PLAYER_INVENTORY_COLUMN_CNT; i++) {
        if (IsKeyPressed(ek_key_code_1 + i, input_state, input_state_last)) {
            world->player_inventory_hotbar_slot_selected = i;
            break;
        }
    }

    switch (input_state->mouse_scroll) {
        case ek_mouse_scroll_state_down:
            world->player_inventory_hotbar_slot_selected++;
            world->player_inventory_hotbar_slot_selected %= PLAYER_INVENTORY_COLUMN_CNT;
            break;

        case ek_mouse_scroll_state_up:
            world->player_inventory_hotbar_slot_selected--;

            if (world->player_inventory_hotbar_slot_selected < 0) {
                world->player_inventory_hotbar_slot_selected += PLAYER_INVENTORY_COLUMN_CNT;
            }

            break;

        default: break;
    }

    assert(world->player_inventory_hotbar_slot_selected >= 0 && world->player_inventory_hotbar_slot_selected < PLAYER_INVENTORY_COLUMN_CNT);
}

void RenderWorld(const s_rendering_context* const rendering_context, const s_world* const world, const s_textures* const textures) {
    RenderClear((s_color){0.2, 0.3, 0.4, 1.0});

    ZeroOut(rendering_context->state->view_mat, sizeof(rendering_context->state->view_mat));
    InitCameraViewMatrix4x4(&rendering_context->state->view_mat, world->cam_pos, rendering_context->display_size);

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

        RenderTilemap(rendering_context, &world->tilemap_activity, tilemap_render_range, textures);
    }

    // Render the player.
    RenderSprite(rendering_context, ek_sprite_player, textures, world->player_pos, PLAYER_ORIGIN, (s_vec_2d){1.0f, 1.0f}, 0.0f, WHITE);

    RenderNPCs(rendering_context, &world->npcs, textures);

    Flush(rendering_context);

#if 0
    const s_vec_2d player_inv_pos = {
        rendering_context->display_size.x * PLAYER_INVENTORY_POS_PERC.x,
        rendering_context->display_size.y * PLAYER_INVENTORY_POS_PERC.y
    };

    const int slot_display_cnt = world->player_inventory_open ? PLAYER_INVENTORY_LENGTH : PLAYER_INVENTORY_COLUMN_CNT;

    for (int i = 0; i < slot_display_cnt; i++) {
        const s_inventory_slot* const slot = &world->player_inventory_slots[i];

        const s_rect slot_rect = {
            player_inv_pos.x + (INVENTORY_SLOT_GAP * (i % PLAYER_INVENTORY_COLUMN_CNT)),
            player_inv_pos.y + ((int)(i / PLAYER_INVENTORY_COLUMN_CNT) * INVENTORY_SLOT_GAP),
            INVENTORY_SLOT_SIZE,
            INVENTORY_SLOT_SIZE
        };

        const s_color slot_outline_color = world->player_inventory_hotbar_slot_selected == i ? YELLOW : WHITE;

        // Render the slot box.
        RenderRect(rendering_context, slot_rect, (s_color){0.0f, 0.0f, 0.0f, PLAYER_INVENTORY_SLOT_BG_ALPHA});
        RenderRectOutline(rendering_context, slot_rect, slot_outline_color, CAMERA_SCALE);

        // Render the item icon.
        if (slot->quantity > 0) {
            RenderSprite(rendering_context, g_items[slot->item_type].spr, textures, RectCenter(slot_rect), (s_vec_2d){0.5f, 0.5f}, (s_vec_2d){CAMERA_SCALE, CAMERA_SCALE}, 0.0f, WHITE);
        }

        // Render the quantity.
        if (slot->quantity > 1) {
            char quant_str_buf[4];
            snprintf(quant_str_buf, sizeof(quant_str_buf), "%d", slot->quantity);

            const s_vec_2d quant_pos = {
                slot_rect.x + slot_rect.width - 14.0f,
                slot_rect.y + slot_rect.height - 6.0f
            };

            RenderStr(rendering_context, quant_str_buf, ek_font_eb_garamond_36, fonts, quant_pos, ek_str_hor_align_right, ek_str_ver_align_bottom, WHITE, temp_mem_arena);
        }
    }
#endif
}

static void RenderInventorySlot(const s_rendering_context* const rendering_context, const s_inventory_slot slot, const s_vec_2d pos, const s_color outline_color, const s_textures* const textures, const s_fonts* const fonts, s_mem_arena* const temp_mem_arena) {
    const s_rect slot_rect = {
        pos.x - (INVENTORY_SLOT_SIZE / 2.0f),
        pos.y - (INVENTORY_SLOT_SIZE / 2.0f),
        INVENTORY_SLOT_SIZE,
        INVENTORY_SLOT_SIZE
    };

    // Render the slot box.
    RenderRect(rendering_context, slot_rect, (s_color){0.0f, 0.0f, 0.0f, PLAYER_INVENTORY_SLOT_BG_ALPHA});
    RenderRectOutline(rendering_context, slot_rect, outline_color, 1.0f);

    // Render the item icon.
    if (slot.quantity > 0) {
        RenderSprite(rendering_context, g_items[slot.item_type].spr, textures, RectCenter(slot_rect), (s_vec_2d){0.5f, 0.5f}, (s_vec_2d){CAMERA_SCALE, CAMERA_SCALE}, 0.0f, WHITE);
    }

    // Render the quantity.
    if (slot.quantity > 1) {
        char quant_str_buf[4];
        snprintf(quant_str_buf, sizeof(quant_str_buf), "%d", slot.quantity);

        const s_vec_2d quant_pos = {
            slot_rect.x + slot_rect.width - 14.0f,
            slot_rect.y + slot_rect.height - 6.0f
        };

        RenderStr(rendering_context, quant_str_buf, ek_font_eb_garamond_36, fonts, quant_pos, ek_str_hor_align_right, ek_str_ver_align_bottom, WHITE, temp_mem_arena);
    }
}

void RenderWorldUI(const s_rendering_context* const rendering_context, const s_world* const world, const s_textures* const textures, const s_fonts* const fonts, s_mem_arena* const temp_mem_arena) {
    const s_vec_2d_i ui_size = UISize(rendering_context->display_size);

    //
    // Player Inventory
    //

    // Draw a backdrop if the player inventory is open.
    if (world->player_inventory_open) {
        const s_vec_2d_i ui_size = UISize(rendering_context->display_size);
        const s_rect bg_rect = {0.0f, 0.0f, ui_size.x, ui_size.y};
        RenderRect(rendering_context, bg_rect, (s_color){0.0f, 0.0f, 0.0f, PLAYER_INVENTORY_BG_ALPHA});
    }

    const float player_inv_mid_x = ui_size.x / 2.0f;

    const float player_inv_left = player_inv_mid_x - (INVENTORY_SLOT_GAP * (PLAYER_INVENTORY_COLUMN_CNT - 1) * 0.5f);

    // Render hotbar.
    const float hotbar_y = ui_size.y - PLAYER_INVENTORY_HOTBAR_BOTTOM_OFFS;

    for (int i = 0; i < PLAYER_INVENTORY_COLUMN_CNT; i++) {
        const s_inventory_slot* const slot = &world->player_inventory_slots[i];
        const float slot_x = player_inv_left + (INVENTORY_SLOT_GAP * i);
        RenderInventorySlot(rendering_context, *slot, (s_vec_2d){slot_x, hotbar_y}, WHITE, textures, fonts, temp_mem_arena);
    }

    // Render the rest of the inventory if open.
    if (world->player_inventory_open) {
        const int player_inv_body_row_cnt = ceilf((float)PLAYER_INVENTORY_LENGTH / PLAYER_INVENTORY_COLUMN_CNT) - 1;
        const float player_inv_body_top = (ui_size.y * PLAYER_INVENTORY_BODY_Y_PERC) - (INVENTORY_SLOT_GAP * (player_inv_body_row_cnt - 1) * 0.5f);

        const int player_inv_body_len = PLAYER_INVENTORY_LENGTH - PLAYER_INVENTORY_COLUMN_CNT;

        for (int i = 0; i < player_inv_body_len; i++) {
            const s_inventory_slot* const slot = &world->player_inventory_slots[PLAYER_INVENTORY_COLUMN_CNT + i];

            const int c = i % PLAYER_INVENTORY_COLUMN_CNT;
            const int r = i / PLAYER_INVENTORY_COLUMN_CNT;

            const s_vec_2d slot_pos = {
                player_inv_left + (INVENTORY_SLOT_GAP * c),
                player_inv_body_top + (INVENTORY_SLOT_GAP * r)
            };

            RenderInventorySlot(rendering_context, *slot, slot_pos, WHITE, textures, fonts, temp_mem_arena);
        }
    }

    //assert(false); // NOTE: Strange issue with pressing escape? Try on low power?
}
