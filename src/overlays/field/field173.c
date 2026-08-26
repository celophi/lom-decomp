#include "common.h"

extern s16 D_80122C10;
extern s32 g_gosub_result_count;

/**
 * @brief Clear D_80122C10 when there are no gosub results.
 */
void func_800C5E08(void)
{
    if (g_gosub_result_count == 0)
    {
        D_80122C10 = 0;
    }
}
