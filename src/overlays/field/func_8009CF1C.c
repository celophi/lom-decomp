#include "common.h"

typedef struct
{
    s32 unk0;  /* 0x00 */
    u8 pad4[0x8 - 0x4];
    s32 unk8;  /* 0x08 */
    u8 padC[0x25 - 0xC];
    u8 unk25;  /* 0x25 */
    u8 pad26[0x30 - 0x26];
} Struct8009CF1C;

/**
 * @brief Tests whether entity @p a1 lies within a bounding box around @p a0.
 *
 * Returns false when @p a1 is inactive (@c unk25 == 0xFF), when the y delta
 * (@c a1->unk8 - @c a0->unk8) is outside [-0x2000, 0x2000], or when the x delta
 * (@c a1->unk0 - @c a0->unk0) is outside [-(t << 8), t << 8]; otherwise true.
 *
 * @param a0 Reference entity (box center).
 * @param a1 Candidate entity to test.
 * @param t Half-width of the x range; scaled by 256.
 * @return 1 when @p a1 is inside the box, 0 otherwise.
 */
s32 func_8009CF1C(Struct8009CF1C *a0, Struct8009CF1C *a1, s32 t)
{
    s32 dy;
    s32 dx;

    if (a1->unk25 == 0xFF)
    {
        return 0;
    }
    dy = a1->unk8 - a0->unk8;
    if (dy >= 0x2001)
    {
        return 0;
    }
    if (dy < -0x2000)
    {
        return 0;
    }
    dx = a1->unk0;
    dx = dx - a0->unk0;
    t <<= 8;
    dy = dx;
    if (t < dy)
    {
        return 0;
    }
    return dy >= -t;
}
