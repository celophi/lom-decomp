#include "common.h"

extern s32 D_801B2810;
extern s32 D_801B2814;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_80082870(void)
{
    D_801B2810 = 1;
    D_801B2814 = 1;
}
