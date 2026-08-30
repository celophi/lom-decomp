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
 * @brief Distance test against three consecutive correction anchors.
 *
 * Loops the corrected GTE distance (D_80105C70[a->unk3A * 0x23C], stepping the
 * s16 correction pair by one each pass) up to three times, returning 1 on the
 * first anchor within @p max_dist.
 *
 * WIP 97.88% (gcc272_cdk). Residue is a 4-row loop-exit branch-polarity /
 * delay-slot layout (target packs i++ into the exit branch's delay slot and
 * branches to the continue path; ours branches to the return). No source shape
 * reproduces it and the permuter cannot run on the GTE inline asm.
 *
 * @return 1 if any anchor is within @p max_dist, else 0 (also 0 when unk25 == 0xFF).
 */
s32 func_8009CF84(RefEntity *a, FieldEntity *b, s32 max_dist)
{
    FieldVector *delta = (FieldVector *)0x1F800000;
    FieldVector *sqr = (FieldVector *)0x1F800010;
    s16 *corr;
    s32 i;
    s32 dist;

    if (b->unk25 == 0xFF)
    {
        return 0;
    }
    corr = (s16 *)&D_80105C70[a->unk3A * 0x23C];
    i = 0;
    do
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
        i++;
        corr += 2;
    } while (i < 3);
    return 0;
}
