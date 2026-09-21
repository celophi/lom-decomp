#include "common.h"

extern s32 D_801B2CE8;
extern s32 D_801B2CEC;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_8009EE94(void)
{
    D_801B2CE8 = 1;
    D_801B2CEC = 1;
}
