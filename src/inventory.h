#ifndef INVENTORY_H
#define INVENTORY_H

#include "game.h"

typedef struct {
    e_item_type item_type;
    int quantity;
} s_inventory_slot;

int AddToInventory(s_inventory_slot* const slots, const int slot_cnt, const e_item_type item_type, int quantity); // Returns the quantity that couldn't be added (0 if everything was added).
int RemoveFromInventory(s_inventory_slot* const slots, const int slot_cnt, const e_item_type item_type, int quantity); // Returns the quantity that couldn't be removed (0 if everything was removed).
bool DoesInventoryHaveRoomFor(s_inventory_slot* const slots, const int slot_cnt, const e_item_type item_type, int quantity);

#endif
