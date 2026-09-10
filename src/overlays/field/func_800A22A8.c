#include "common.h"

extern s32 D_80117E78;

/**
 * @brief Evaluate fixed-point interpolation weights for a knot sequence.
 * @param order Number of interpolation levels, including the initial span test.
 * @param parameter Parameter to evaluate against the knot sequence.
 * @param span Receives the selected knot span.
 * @param knots Knot values used by the interpolation recurrence.
 * @param workspace Interleaved two-column weight buffer for successive levels.
 * @note Weights use 0x1000 as unity and alternate columns on each level.
 * @note Keep separate knot pairs to preserve the original register allocation.
 * @note GCC 2.7.2 CDK matches all 173 instructions (692 bytes).
 */
void func_800A22A8(s32 order, s32 parameter, s32 *span, s32 *knots, s32 *workspace)
{
    s32 i;
    s32 level;
    s32 left;
    s32 right;
    s32 lower;
    s32 upper;
    s32 second_lower;
    s32 second_upper;

    for (i = 0; i < D_80117E78 + 1; i++)
    {
        ((s32 (*)[2])workspace)[i][0] = 0;
    }
    for (i = 0; i < D_80117E78; i++)
    {
        if (parameter >= knots[i] && parameter < knots[i + 1])
        {
            ((s32 (*)[2])workspace)[i][0] = 0x1000;
            *span = i;
        }
    }
    if (parameter >= knots[D_80117E78 - 1] && parameter <= knots[D_80117E78] + 1)
    {
        ((s32 (*)[2])workspace)[D_80117E78 - 1][0] = 0x1000;
        *span = D_80117E78 - 1;
    }
    for (level = 1; level < order; level++)
    {
        for (i = 0; i < D_80117E78 + 1; i++)
        {
            ((s32 (*)[2])workspace)[i][level & 1] = 0;
        }
        for (i = *span - level; i <= *span; i++)
        {
            right = 0;
            left = right;
            lower = knots[i + 1];
            upper = knots[i + level + 1];
            if (lower != upper)
            {
                right = ((upper - parameter) * ((s32 (*)[2])workspace)[i + 1][(level - 1) & 1]) / (upper - lower);
            }
            second_lower = knots[i];
            second_upper = knots[i + level];
            if (second_lower != second_upper)
            {
                left = ((parameter - second_lower) * ((s32 (*)[2])workspace)[i][(level - 1) & 1]) / (second_upper - second_lower);
            }
            ((s32 (*)[2])workspace)[i][level & 1] = right + left;
        }
    }
}
