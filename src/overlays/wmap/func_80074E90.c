#include "common.h"

extern s32 D_801B2580;
extern s32 D_801B2584;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_80074E90(void)
{
    D_801B2580 = 1;
    D_801B2584 = 1;
}
