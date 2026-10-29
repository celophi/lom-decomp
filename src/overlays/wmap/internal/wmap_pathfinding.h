#ifndef WMAP_PATHFINDING_H
#define WMAP_PATHFINDING_H

#include "common.h"

void wmap_unlock_land_encyclopedia_entries(void);
void wmap_commit_land_entry(s32 cell_x, s32 cell_y);
void func_8005EB68(s32 start_x, s32 start_y, s32 end_x, s32 end_y, s32* out_x, s32* out_y);

#endif
