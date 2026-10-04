#ifndef FIELD_EQUIPMENT_COMBINATION_RULES_H
#define FIELD_EQUIPMENT_COMBINATION_RULES_H

#include "common.h"

/**
 * @brief Find the first golem logic-block recipe made by an equipment pair.
 * @param record_indices Two inventory indices supplying the equipment.
 * @param level Receives the block level when a recipe matches.
 * @param shape Receives the block shape when a recipe matches.
 * @return Logic-block id, or zero when no recipe matches.
 */
s32 golem_find_logic_block_recipe(s32* record_indices, s32* level, s32* shape);

#endif /* FIELD_EQUIPMENT_COMBINATION_RULES_H */
