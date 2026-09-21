#include "common.h"

extern u32 D_801B2BD0;
extern s32 D_801B2BD4;
extern void (*D_800D6388[])(void);
extern void func_8006A2FC(u8* a0, u8* a1, s32 a2, s32 a3, s32 a4, s32 a5, s32 a6, s32 a7);
extern u8 D_800D9688[];
extern u8 D_80139A48[];
extern s32 D_80139280;

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_80097234(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2BD0 = 1;
        D_801B2BD4 = 1;
        return 1;
    }

    if (D_801B2BD0 < 0x6)
    {
        D_800D6388[D_801B2BD0]();
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
void func_800972AC(void)
{
    D_801B2BD0 = 1;
    D_801B2BD4 = 1;
}

/**
 * @brief Draw the world-map sprite via func_8006A2FC, then advance after the wait expires.
 */
void func_800972C4(void)
{
    func_8006A2FC(D_800D9688, D_80139A48, 0xC, 0, 0x7F, 0x2, 0, (s32)((u8*)D_80139280 + 0x78));
    if (--D_801B2BD4 == 0)
    {
        D_801B2BD0 += 1;
    }
}
