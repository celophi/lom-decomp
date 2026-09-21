#include "common.h"

extern u32 D_801B2940;
extern s32 D_801B2944;
extern void (*D_800D5B18[])(void);
extern void func_8006A2FC(u8* a0, u8* a1, s32 a2, s32 a3, s32 a4, s32 a5, s32 a6, s32 a7);
extern u8 D_800D9F20[];
extern u8 D_80139BD8[];
extern s32 D_80139280;

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_80088350(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2940 = 1;
        D_801B2944 = 1;
        return 1;
    }

    if (D_801B2940 < 0x6)
    {
        D_800D5B18[D_801B2940]();
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
void func_800883C8(void)
{
    D_801B2940 = 1;
    D_801B2944 = 1;
}

/**
 * @brief Draw the world-map sprite via func_8006A2FC, then advance after the wait expires.
 */
void func_800883E0(void)
{
    func_8006A2FC(D_800D9F20, D_80139BD8, 0x32, 0, 0x7F, 0x2, 0, (s32)((u8*)D_80139280 + 0x28));
    if (--D_801B2944 == 0)
    {
        D_801B2940 += 1;
    }
}
