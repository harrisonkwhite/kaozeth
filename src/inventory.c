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
        pos.x, pos.y,
        INVENTORY_SLOT_SIZE, INVENTORY_SLOT_SIZE
    };

    // Render the slot box.
    RenderRect(rendering_context, slot_rect, (s_color){0.0f, 0.0f, 0.0f, INVENTORY_SLOT_BG_ALPHA});
    RenderRectOutline(rendering_context, slot_rect, outline_color, 1.0f);

    // Render the item icon.
    if (slot.quantity > 0) {
        RenderSprite(rendering_context, g_item_types[slot.item_type].icon_spr, textures, RectCenter(slot_rect), (s_vec_2d){0.5f, 0.5f}, (s_vec_2d){CAMERA_SCALE / UI_SCALE, CAMERA_SCALE / UI_SCALE}, 0.0f, WHITE);
    }

    return true;
}

static s_vec_2d PlayerInventorySlotPos(const int r, const int c, const s_vec_2d_i ui_size) {
    assert(r >= 0 && r < PLAYER_INVENTORY_ROW_CNT);
    assert(c >= 0 && c < PLAYER_INVENTORY_COLUMN_CNT);
    assert(ui_size.x > 0 && ui_size.y > 0);

    const s_vec_2d top_left = {ui_size.x * PLAYER_INVENTORY_POS_PERC.x, ui_size.y * PLAYER_INVENTORY_POS_PERC.y};

    return (s_vec_2d){
        top_left.x + (INVENTORY_SLOT_GAP * c),
        top_left.y + (INVENTORY_SLOT_GAP * r)
    };
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

static void WriteItemNameStr(char* const str_buf, const int str_buf_size, const e_item_type item_type, const int quantity) {
    assert(str_buf);
    assert(str_buf_size > 0);
    assert(quantity >= 1);

    if (quantity == 1) {
        snprintf(str_buf, str_buf_size, "%s", g_item_types[item_type].name);
    } else {
        snprintf(str_buf, str_buf_size, "%s (%d)", g_item_types[item_type].name, quantity);
    }
}

void ProcPlayerInventoryOpenState(s_world* const world, const s_input_state* const input_state, const s_input_state* const input_state_last, const s_vec_2d_i display_size) {
    assert(world->player_inv_open);

    const s_vec_2d_i ui_size = UISize(display_size);
    const s_vec_2d cursor_ui_pos = DisplayToUIPos(input_state->mouse_pos);

    for (int r = 0; r < PLAYER_INVENTORY_ROW_CNT; r++) {
        for (int c = 0; c < PLAYER_INVENTORY_COLUMN_CNT; c++) {
            s_inventory_slot* const slot = &world->player_inv_slots[r][c];

            const s_vec_2d slot_pos = PlayerInventorySlotPos(r, c, ui_size);

            const s_rect slot_collider = {
                slot_pos.x,
                slot_pos.y,
                INVENTORY_SLOT_SIZE,
                INVENTORY_SLOT_SIZE
            };

            if (IsPointInRect(cursor_ui_pos, slot_collider)) {
                if (slot->quantity > 0) {
                    // Update cursor hover string with slot item.
                    WriteItemNameStr(world->cursor_hover_str, sizeof(world->cursor_hover_str), slot->item_type, slot->quantity);
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
}

bool RenderPlayerInventory(const s_rendering_context* const rendering_context, const s_world* const world, const s_textures* const textures, const s_fonts* const fonts, s_mem_arena* const temp_mem_arena) {
    const s_vec_2d_i ui_size = UISize(rendering_context->display_size);

    if (world->player_inv_open) {
        // Draw a backdrop.
        const s_vec_2d_i ui_size = UISize(rendering_context->display_size);
        const s_rect bg_rect = {0.0f, 0.0f, ui_size.x, ui_size.y};
        RenderRect(rendering_context, bg_rect, (s_color){0.0f, 0.0f, 0.0f, PLAYER_INVENTORY_BG_ALPHA});
    } else {
        // Render current item name.
        const s_inventory_slot* const slot = &world->player_inv_slots[0][world->player_inv_hotbar_slot_selected];

        if (slot->quantity > 0) {
            char name_buf[32];
            WriteItemNameStr(name_buf, sizeof(name_buf), slot->item_type, slot->quantity);

            const s_vec_2d name_pos = {
                ui_size.x * PLAYER_INVENTORY_POS_PERC.x,
                (ui_size.y * PLAYER_INVENTORY_POS_PERC.y) + INVENTORY_SLOT_SIZE + 8.0f
            };

            if (!RenderStr(rendering_context, name_buf, ek_font_eb_garamond_24, fonts, name_pos, ek_str_hor_align_left, ek_str_ver_align_top, WHITE, temp_mem_arena)) {
                return false;
            }
        }
    }

    // Draw inventory slots.
    const int row_cnt = world->player_inv_open ? PLAYER_INVENTORY_ROW_CNT : 1;

    for (int r = 0; r < row_cnt; r++) {
        for (int c = 0; c < PLAYER_INVENTORY_COLUMN_CNT; c++) {
            const s_inventory_slot* const slot = &world->player_inv_slots[r][c];
            const s_vec_2d slot_pos = PlayerInventorySlotPos(r, c, ui_size);
            const s_color slot_color = r == 0 && c == world->player_inv_hotbar_slot_selected ? YELLOW : WHITE;

            if (!RenderInventorySlot(rendering_context, *slot, slot_pos, slot_color, textures, fonts, temp_mem_arena)) {
                return false;
            }
        }
    }

    return true;
}
