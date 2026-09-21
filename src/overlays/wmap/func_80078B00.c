#include "common.h"

extern s32 D_801B2640;
extern s32 D_801B2644;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_80078B00(void)
{
    D_801B2640 = 1;
    D_801B2644 = 1;
}
