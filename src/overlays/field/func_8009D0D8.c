#include "common.h"
#include "field_types.h"
#include "sdk/libgte.h"
#include "sdk/inline_c.h"
#include "sdk/gte_dmpsx_compat.h"

typedef struct
{
    s32 vx;
    s32 vy;
    s32 vz;
    u8 unk0C[0x2E];
    u8 unk3A;
} RefEntity;

extern u8 D_80105C70[];

/**
 * @brief Distance test with a per-index x/z correction from D_80105C70.
 * @param a Reference entity; unk3A selects a 0x23C-stride correction record.
 * @param b Target entity; skipped when its unk25 flag is 0xFF.
 * @param max_dist Maximum corrected distance for a positive result.
 * @return 1 if the corrected GTE distance is below @p max_dist, else 0.
 */
s32 func_8009D0D8(RefEntity *a, FieldEntity *b, s32 max_dist)
{
    FieldVector *delta = (FieldVector *)0x1F800000;
    FieldVector *sqr = (FieldVector *)0x1F800010;
    s16 *corr;
    s32 dist;

    corr = (s16 *)&D_80105C70[a->unk3A * 0x23C];
    if (b->unk25 != 0xFF)
    {
        delta->vx = ((b->vx - a->vx) - (corr[0] << 8)) >> 8;
        delta->vy = (b->vy - a->vy) >> 8;
        delta->vz = ((b->vz - a->vz) - (corr[1] << 8)) >> 8;
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
