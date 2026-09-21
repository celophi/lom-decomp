#include "common.h"

extern u32 D_801B28E0;
extern s32 D_801B28E4;
extern void (*D_800D59F8[])(void);
extern void func_8006A2FC(u8* a0, u8* a1, s32 a2, s32 a3, s32 a4, s32 a5, s32 a6, s32 a7);
extern u8 D_800DA448[];
extern u8 D_80139CC8[];
extern s32 D_80139280;

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_800866E8(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B28E0 = 1;
        D_801B28E4 = 1;
        return 1;
    }

    if (D_801B28E0 < 0x6)
    {
        D_800D59F8[D_801B28E0]();
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
void func_80086760(void)
{
    D_801B28E0 = 1;
    D_801B28E4 = 1;
}

/**
 * @brief Draw the world-map sprite via func_8006A2FC, then advance after the wait expires.
 */
void func_80086778(void)
{
    func_8006A2FC(D_800DA448, D_80139CC8, 0xA, 0, 0x7F, 0x2, 0, (s32)((u8*)D_80139280 + 0x28));
    if (--D_801B28E4 == 0)
    {
        D_801B28E0 += 1;
    }
}
