#ifndef FIELD_GOLEM_LOGIC_BLOCKS_H
#define FIELD_GOLEM_LOGIC_BLOCKS_H

#include "common.h"

/** @brief Placement status and icon palette of one golem logic block. */
typedef struct
{
    u16 is_unavailable; /**< Non-zero when the block cannot join the edited group. */
    u16 clut;           /**< CLUT row used to draw the block's icon. */
} GolemLogicBlockStatus;

void golem_logic_block_append(u32 block_id, u32 level, u32 shape);
u32 golem_rebuild_grid_owners(void);
void golem_place_logic_block(s32 index, s32 rotation, s32 x, s32 y);
s32 golem_can_place_logic_block(s32 index, s32 rotation, s32 x, s32 y);
s32 golem_logic_block_fits_grid(s32 index, s32 rotation, s32 x, s32 y);
s32 golem_fill_logic_block_status(GolemLogicBlockStatus* results);
void golem_remove_logic_block(s32 index);
void golem_build_grid_markers(s32* markers);

#endif /* FIELD_GOLEM_LOGIC_BLOCKS_H */
