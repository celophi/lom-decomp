#include "common.h"

extern u8 g_menuLayoutBuffer[];
extern s32 D_80122C00;
extern s16 D_80122C10;
extern s32 g_gosub_result_count;

extern void func_800B2844(s32 arg0, u8 *arg1, u8 arg2);

void func_800C5DA8(void)
{
    u8 idx = g_menuLayoutBuffer[D_80122C00 + 0x29D8];

    func_800B2844(0, &g_menuLayoutBuffer[idx * 332 + 0x2B0C], 0xFF);
}

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
