#include "wmap_effect_primitives.h"
#include "common.h"

extern u32 D_801B29A0;
extern s32 D_801B29A4;
extern void (*D_800D5C68[])(void);
extern u8 D_800D95D8[];
extern u8 D_80139A28[];
extern s32 D_80139280;

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_8008A738(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B29A0 = 1;
        D_801B29A4 = 1;
        return 1;
    }

    if (D_801B29A0 < 0x4)
    {
        D_800D5C68[D_801B29A0]();
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
void func_8008A7B0(void)
{
    D_801B29A0 = 1;
    D_801B29A4 = 1;
}

/**
 * @brief Draw the world-map sprite via func_8006A2FC, then advance after the wait expires.
 */
void func_8008A7C8(void)
{
    func_8006A2FC(D_800D95D8, D_80139A28, 0xC, 0, 0x7F, 0x2, 0, (s32)((u8*)D_80139280 + 0x50));
    if (--D_801B29A4 == 0)
    {
        D_801B29A0 += 1;
    }
}
