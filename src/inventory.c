#include "inventory.h"

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
