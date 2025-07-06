#include <stdio.h>
#include "game.h"

int AddToInventory(s_inventory_slot* const slots, const int slot_cnt, const e_item_type item_type, int quantity) {
    assert(slots);
    assert(slot_cnt > 0);
    assert(quantity > 0);

    for (int i = 0; i < slot_cnt && quantity > 0; i++) {
        if (slots[i].quantity > 0 && slots[i].item_type == item_type && slots[i].quantity < ITEM_QUANTITY_LIMIT) {
            const int quant_to_add = MIN(ITEM_QUANTITY_LIMIT - slots[i].quantity, quantity);
            slots[i].quantity += quant_to_add;
            quantity -= quant_to_add;
        }
    }

    for (int i = 0; i < slot_cnt && quantity > 0; i++) {
        if (slots[i].quantity == 0) {
            const int quant_to_add = MIN(ITEM_QUANTITY_LIMIT, quantity);
            slots[i].item_type = item_type;
            slots[i].quantity += quant_to_add;
            quantity -= quant_to_add;
        }
    }

    assert(quantity >= 0);
    return quantity;
}

int RemoveFromInventory(s_inventory_slot* const slots, const int slot_cnt, const e_item_type item_type, int quantity) {
    assert(slots);
    assert(slot_cnt > 0);
    assert(quantity > 0);

    for (int i = 0; i < slot_cnt && quantity > 0; i++) {
        if (slots[i].quantity > 0 && slots[i].item_type == item_type) {
            const int quant_to_remove = MIN(slots[i].quantity, quantity);
            slots[i].quantity -= quant_to_remove;
            quantity -= quant_to_remove;
        }
    }

    return quantity;
}

bool DoesInventoryHaveRoomFor(s_inventory_slot* const slots, const int slot_cnt, const e_item_type item_type, int quantity) {
    for (int i = 0; i < slot_cnt && quantity > 0; i++) {
        s_inventory_slot* const slot = &slots[i];

        if (slot->quantity == 0 || slot->item_type == item_type) {
            const int remaining = ITEM_QUANTITY_LIMIT - slot->quantity;
            quantity = MAX(quantity - remaining, 0);
        }
    }

    assert(quantity >= 0);
    return quantity == 0;
}

bool RenderInventorySlot(const s_rendering_context* const rendering_context, const s_inventory_slot slot, const s_vec_2d pos, const s_color outline_color, const s_textures* const textures, const s_fonts* const fonts, s_mem_arena* const temp_mem_arena) {
    const s_rect slot_rect = {
        pos.x - (INVENTORY_SLOT_SIZE / 2.0f),
        pos.y - (INVENTORY_SLOT_SIZE / 2.0f),
        INVENTORY_SLOT_SIZE,
        INVENTORY_SLOT_SIZE
    };

    // Render the slot box.
    RenderRect(rendering_context, slot_rect, (s_color){0.0f, 0.0f, 0.0f, INVENTORY_SLOT_BG_ALPHA});
    RenderRectOutline(rendering_context, slot_rect, outline_color, 1.0f);

    // Render the item icon.
    if (slot.quantity > 0) {
        RenderSprite(rendering_context, g_item_types[slot.item_type].icon_spr, textures, RectCenter(slot_rect), (s_vec_2d){0.5f, 0.5f}, (s_vec_2d){CAMERA_SCALE / UI_SCALE, CAMERA_SCALE / UI_SCALE}, 0.0f, WHITE);
    }

    // Render the quantity.
    if (slot.quantity > 1) {
        char quant_str_buf[4];
        snprintf(quant_str_buf, sizeof(quant_str_buf), "%d", slot.quantity);

        const s_vec_2d quant_pos = {
            slot_rect.x + (slot_rect.width / 2.0f),
            slot_rect.y + slot_rect.height - 2.0f
        };

        if (!RenderStr(rendering_context, quant_str_buf, ek_font_eb_garamond_24, fonts, quant_pos, ek_str_hor_align_center, ek_str_ver_align_bottom, WHITE, temp_mem_arena)) {
            return false;
        }
    }

    return true;
}

void LoadPlayerInventorySlotPositions(s_vec_2d (* const positions)[PLAYER_INVENTORY_LENGTH], const s_vec_2d_i ui_size) {
    assert(positions);
    assert(ui_size.x > 0 && ui_size.y > 0);

    const float mid_x = ui_size.x / 2.0f;
    const float left = mid_x - (INVENTORY_SLOT_GAP * (PLAYER_INVENTORY_COLUMN_CNT - 1) * 0.5f);

    //
    // Hotbar
    //
    const float hotbar_y = ui_size.y - PLAYER_INVENTORY_HOTBAR_BOTTOM_OFFS;

    for (int i = 0; i < PLAYER_INVENTORY_COLUMN_CNT; i++) {
        (*positions)[i] = (s_vec_2d){
            left + (INVENTORY_SLOT_GAP * i),
            hotbar_y
        };
    }

    //
    // Body
    //
    const int body_row_cnt = ceilf((float)PLAYER_INVENTORY_LENGTH / PLAYER_INVENTORY_COLUMN_CNT) - 1;
    const float top = (ui_size.y * PLAYER_INVENTORY_BODY_Y_PERC) - (INVENTORY_SLOT_GAP * (body_row_cnt - 1) * 0.5f);

    for (int i = PLAYER_INVENTORY_COLUMN_CNT; i < PLAYER_INVENTORY_LENGTH; i++) {
        const int bi = i - PLAYER_INVENTORY_COLUMN_CNT;
        const int c = bi % PLAYER_INVENTORY_COLUMN_CNT;
        const int r = bi / PLAYER_INVENTORY_COLUMN_CNT;

        (*positions)[i] = (s_vec_2d){
            left + (INVENTORY_SLOT_GAP * c),
            top + (INVENTORY_SLOT_GAP * r)
        };
    }
}

void UpdatePlayerInventoryHotbarSlotSelected(int* const hotbar_slot_selected, const s_input_state* const input_state, const s_input_state* const input_state_last) {
    assert(hotbar_slot_selected);
    assert(*hotbar_slot_selected >= 0 && *hotbar_slot_selected < PLAYER_INVENTORY_COLUMN_CNT);

    for (int i = 0; i < PLAYER_INVENTORY_COLUMN_CNT; i++) {
        if (IsKeyPressed(ek_key_code_1 + i, input_state, input_state_last)) {
            *hotbar_slot_selected = i;
            break;
        }
    }

    if (input_state->mouse_scroll_state == ek_mouse_scroll_state_down) {
        (*hotbar_slot_selected)++;
        (*hotbar_slot_selected) %= PLAYER_INVENTORY_COLUMN_CNT;
    } else if (input_state->mouse_scroll_state == ek_mouse_scroll_state_up) {
        (*hotbar_slot_selected)--;

        if (*hotbar_slot_selected < 0) {
            *hotbar_slot_selected += PLAYER_INVENTORY_COLUMN_CNT;
        }
    }

    assert(*hotbar_slot_selected >= 0 && *hotbar_slot_selected < PLAYER_INVENTORY_COLUMN_CNT);
}

void ProcPlayerInventoryOpenState(s_world* const world, const s_input_state* const input_state, const s_input_state* const input_state_last, const s_vec_2d_i display_size) {
    assert(world->player_inv_open);

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
            if (slot->quantity > 0) {
                // Update cursor hover string with slot item.
                if (slot->quantity == 1) {
                    snprintf(world->cursor_hover_str, sizeof(world->cursor_hover_str), "%s", g_item_types[slot->item_type].name);
                } else {
                    snprintf(world->cursor_hover_str, sizeof(world->cursor_hover_str), "%s (%d)", g_item_types[slot->item_type].name, slot->quantity);
                }
            }

            // Handle slot click event.
            const bool clicked = IsMouseButtonPressed(ek_mouse_button_code_left, input_state, input_state_last);

            if (clicked) {
                if (slot->quantity > 0 && world->cursor_item_held_quantity > 0 && slot->item_type == world->cursor_item_held_type) {
                    const int to_add = MIN(world->cursor_item_held_quantity, ITEM_QUANTITY_LIMIT - slot->quantity);

                    if (to_add == 0) {
                        const e_item_type item_type_temp = slot->item_type;
                        const int quantity_temp = slot->quantity;

                        slot->item_type = world->cursor_item_held_type;
                        slot->quantity = world->cursor_item_held_quantity;

                        world->cursor_item_held_type = item_type_temp;
                        world->cursor_item_held_quantity = quantity_temp;
                    } else {
                        slot->quantity += to_add;
                        world->cursor_item_held_quantity -= to_add;
                    }
                } else {
                    const e_item_type item_type_temp = slot->item_type;
                    const int quantity_temp = slot->quantity;

                    slot->item_type = world->cursor_item_held_type;
                    slot->quantity = world->cursor_item_held_quantity;

                    world->cursor_item_held_type = item_type_temp;
                    world->cursor_item_held_quantity = quantity_temp;
                }
            }

            break;
        }
    }
}
