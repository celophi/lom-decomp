#include "common.h"

extern u32 D_801B2A90;
extern s32 D_801B2A94;
extern void (*D_800D5F90[])(void);
extern void func_8006A2FC(u8* a0, u8* a1, s32 a2, s32 a3, s32 a4, s32 a5, s32 a6, s32 a7);
extern u8 D_800D99F8[];
extern u8 D_80139AE8[];
extern s32 D_80139280;

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_80090AF4(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2A90 = 1;
        D_801B2A94 = 1;
        return 1;
    }

    if (D_801B2A90 < 0x6)
    {
        D_800D5F90[D_801B2A90]();
        result = 1;
    }
    else
    {
        result = 0;
    }
    return result;
}

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_80090B6C(void)
{
    D_801B2A90 = 1;
    D_801B2A94 = 1;
}

/**
 * @brief Draw the world-map sprite via func_8006A2FC, then advance after the wait expires.
 */
void func_80090B84(void)
{
    func_8006A2FC(D_800D99F8, D_80139AE8, 0xA, 0, 0x7F, 0x2, 0, (s32)((u8*)D_80139280 + 0x28));
    if (--D_801B2A94 == 0)
    {
        D_801B2A90 += 1;
    }
}
