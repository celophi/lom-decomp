#include "common.h"

extern s32 D_8011F350;
extern s32 D_8011F3A8;
extern s32 D_8011F3AC;
extern s32 D_8011F3B4;
extern s32 D_8011F3BC;

/**
 * @brief Advance the field fade-in ramp by one step, or finish it.
 *
 * Marks the ramp active, then eases @c D_8011F350 toward its target by the
 * remaining-steps fraction. Once @c D_8011F3A8 reaches 0x10 the ramp completes:
 * the step counter and phase are reset and the routine returns early; otherwise
 * it decrements @c D_8011F3BC and advances the step counter.
 *
 * @see decomp.me (100%) TODO
 */
void func_800A496C(void)
{
    D_8011F3B4 = 1;
    if (D_8011F3A8 < 0x10)
    {
        D_8011F350 -= D_8011F350 / (0x10 - D_8011F3A8);
    }
    else
    {
        D_8011F3A8 = 0;
        D_8011F3AC = 0;
        return;
    }
    D_8011F3BC -= 8;
    D_8011F3A8 += 1;
}
