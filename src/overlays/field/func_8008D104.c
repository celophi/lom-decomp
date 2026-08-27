#include "common.h"

/**
 * @brief World-space point pair used for the heading lookup.
 */
typedef struct
{
    s32 unk0;
    s32 unk4;
    s32 unk8;
} PointD104;

extern s32 D_800EB0A4[];
extern long ratan2(long y, long x);

/**
 * @brief Maps the heading from @p b to @p a onto a direction-table entry.
 *
 * Computes the ratan2 angle between the two points, quantizes it to a 256-step
 * heading wrapped into [0, 0x100), and returns D_800EB0A4[heading >> 5] plus 10.
 */
s32 func_8008D104(PointD104 *a, PointD104 *b)
{
    s32 angle;
    s32 idx;

    angle = ratan2(a->unk8 - b->unk8, b->unk0 - a->unk0) >> 4;
    idx = angle + 0x10;
    if (idx < 0)
    {
        idx = angle + 0x110;
    }
    if (idx >= 0x100)
    {
        idx -= 0x100;
    }
    return D_800EB0A4[idx >> 5] + 0xA;
}
