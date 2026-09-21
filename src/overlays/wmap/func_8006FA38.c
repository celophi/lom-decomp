#include "common.h"

extern s32 D_801B2440;
extern s32 D_801B2444;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_8006FA38(void)
{
    D_801B2440 = 1;
    D_801B2444 = 1;
}
