#include "common.h"

extern s32 D_801B2CF8;
extern s32 D_801B2CFC;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_8009F140(void)
{
    D_801B2CF8 = 1;
    D_801B2CFC = 1;
}
