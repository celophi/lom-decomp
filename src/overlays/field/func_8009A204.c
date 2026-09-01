#include "common.h"
#include "field_types.h"
#include "sdk/libgte.h"
#include "sdk/inline_c.h"
#include "sdk/gte_dmpsx_compat.h"

/**
 * @brief Return the GTE-computed distance between positions @p a and @p b.
 * @param a First position (vx/vy/vz).
 * @param b Second position (vx/vy/vz).
 * @return sqrt(sum of squared per-axis deltas), each delta scaled by >> 8.
 */
s32 func_8009A204(FieldVector *a, FieldVector *b)
{
    FieldVector *delta = (FieldVector *)0x1F800080;
    FieldVector *sqr = (FieldVector *)0x1F800090;

    delta->vx = (a->vx - b->vx) >> 8;
    delta->vy = (a->vy - b->vy) >> 8;
    delta->vz = (a->vz - b->vz) >> 8;
    gte_ldlvl(delta);
    gte_sqr0();
    gte_stlvnl(sqr);
    return SquareRoot0(sqr->vx + sqr->vy + sqr->vz);
}
