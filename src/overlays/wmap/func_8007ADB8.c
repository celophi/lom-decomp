#include "common.h"

extern s32 D_801B26D0;
extern s32 D_801B26D4;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_8007ADB8(void)
{
    D_801B26D0 = 1;
    D_801B26D4 = 1;
}
