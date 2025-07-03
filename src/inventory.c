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
    RenderRect(rendering_context, slot_rect, (s_color){0.0f, 0.0f, 0.0f, PLAYER_INVENTORY_SLOT_BG_ALPHA});
    RenderRectOutline(rendering_context, slot_rect, outline_color, 1.0f);

    // Render the item icon.
    if (slot.quantity > 0) {
        RenderSprite(rendering_context, g_item_types[slot.item_type].spr, textures, RectCenter(slot_rect), (s_vec_2d){0.5f, 0.5f}, (s_vec_2d){1.0f, 1.0f}, 0.0f, WHITE);
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
