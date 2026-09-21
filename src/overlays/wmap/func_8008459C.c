#include "common.h"

extern s32 D_801B2870;
extern s32 D_801B2874;

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_8008459C(void)
{
    D_801B2870 = 1;
    D_801B2874 = 1;
}
