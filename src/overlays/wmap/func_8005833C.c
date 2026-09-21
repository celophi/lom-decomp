#include "common.h"

/** @brief Per-cell animation state and frame counters. */
typedef struct
{
    s16 unk00;
    s16 unk02;
    s16 state;
    s16 frame;
    s32 unk08;
    s32 unk0c;
    s16 unk10;
    u8 pad12[2];
    s32 unk14;
    s32 unk18;
} WmapEffectCell;

extern WmapEffectCell D_8011D108[6][6];

/**
 * @brief Select a cell effect transition when its requested mode changes.
 * @param x Map-grid X coordinate.
 * @param y Map-grid Y coordinate.
 * @param mode Requested mode: zero, one, or two.
 */
void func_8005833C(s32 x, s32 y, s32 mode)
{
    WmapEffectCell *cell;

    cell = &D_8011D108[x][y];
    if (mode != cell->unk02)
    {
        switch (cell->unk02)
        {
        case 0:
            cell->unk02 = mode;
            if (mode == 1)
            {
                cell->state = mode;
                return;
            }
            cell->state = 3;
            return;
        case 1:
            cell->unk02 = mode;
            if (mode != 0)
            {
                cell->state = 3;
                return;
            }
            cell->state = 0;
            cell->frame = 0;
            return;
        case 2:
            cell->unk02 = mode;
            if (mode != 0)
            {
                cell->state = 4;
                return;
            }
            cell->state = 0;
            cell->frame = 0;
            return;
        }
    }
}
