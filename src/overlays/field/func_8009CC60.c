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
 * @brief Test whether entity @p b is within @p max_dist of position @p a.
 * @param a Reference position (vx/vy/vz).
 * @param b Target entity; skipped when its unk25 flag is 0xFF.
 * @param max_dist Maximum distance for a positive result.
 * @return 1 if the GTE-computed distance is below @p max_dist, else 0.
 */
s32 func_8009CC60(FieldVector *a, FieldEntity *b, s32 max_dist)
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
        if (dist < max_dist)
        {
            return 1;
        }
    }
    return 0;
}
