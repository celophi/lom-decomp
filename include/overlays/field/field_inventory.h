#ifndef FIELD_INVENTORY_H
#define FIELD_INVENTORY_H

#include "common.h"

#include "common/saved_game.h"

/** @brief Close gaps in the saved inventory and mark the remaining records free. */
void field_compact_inventory(void);

/**
 * @brief Copy one item record.
 * @param destination Record receiving the copy.
 * @param source Record to copy.
 */
void field_copy_inventory_record(FieldItemRecord* destination, FieldItemRecord* source);

/**
 * @brief Find a free record in the saved inventory.
 * @return The first free record, or NULL when the inventory is full.
 */
FieldItemRecord* field_find_free_inventory_record(void);

#endif
