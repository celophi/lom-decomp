#include "common.h"

extern s32 D_801B2578;
extern s32 D_801B257C;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_80075774(void)
{
    D_801B2578 = 1;
    D_801B257C = 1;
}
