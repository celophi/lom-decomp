#include "common.h"
#include "sdk/libgte.h"
#include "sdk/inline_c.h"
#include "sdk/gte_dmpsx_compat.h"

typedef struct
{
    s32 vx;
    s32 vy;
    s32 vz;
    s32 pad;
} FieldVector;

typedef struct
{
    s32 vx;
    s32 vy;
    s32 vz;
    u8 unk0C[0x19];
    u8 unk25;
} FieldEntity;

/**
 * @brief Test whether entity @p b sits in the annulus around position @p a.
 * @param a Reference position (vx/vy/vz).
 * @param b Target entity; skipped when its unk25 flag is 0xFF.
 * @param max_dist Outer band is @p max_dist + 0x40; inner band is 0x40.
 * @return 1 if 0x40 < distance < @p max_dist + 0x40, else 0.
 */
s32 func_8009CD30(FieldVector *a, FieldEntity *b, s32 max_dist)
{
    FieldVector *delta = (FieldVector *)0x1F800000;
    FieldVector *sqr = (FieldVector *)0x1F800010;
    s32 dist;

    if (b->unk25 != 0xFF)
    {
        delta->vx = (b->vx - a->vx) >> 8;
        delta->vy = (b->vy - a->vy) >> 8;
        delta->vz = (b->vz - a->vz) >> 8;
        gte_ldlvl(delta);
        gte_sqr0();
        gte_stlvnl(sqr);
        dist = SquareRoot0(sqr->vx + sqr->vy + sqr->vz);
        if (dist < max_dist + 0x40)
        {
            if (dist > 0x40)
            {
                return 1;
            }
        }
    }
    return 0;
}
