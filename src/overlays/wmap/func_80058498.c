#include "common.h"

extern s32 D_800D7D60;
extern s32 D_800D7D64;
extern s32 D_800D7D68;

/**
 * @brief Clear three world-map state values.
 */
void func_80058498(void)
{
    D_800D7D60 = 0;
    D_800D7D64 = 0;
    D_800D7D68 = 0;
}
