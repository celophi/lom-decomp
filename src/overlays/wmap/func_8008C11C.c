#include "wmap_effect_primitives.h"
#include "common.h"

extern u32 D_801B29E0;
extern s32 D_801B29E4;
extern void (*D_800D5D38[])(void);
extern u8 D_800D9B00[];
extern u8 D_80139B18[];
extern s32 D_80139280;

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_8008C0A4(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B29E0 = 1;
        D_801B29E4 = 1;
        return 1;
    }

    if (D_801B29E0 < 0x4)
    {
        D_800D5D38[D_801B29E0]();
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
void func_8008C11C(void)
{
    D_801B29E0 = 1;
    D_801B29E4 = 1;
}

/**
 * @brief Draw the world-map sprite via func_8006A2FC, then advance after the wait expires.
 */
void func_8008C134(void)
{
    func_8006A2FC(D_800D9B00, D_80139B18, 0x3, 0, 0x7F, 0x2, 0, (s32)((u8*)D_80139280 + 0x28));
    if (--D_801B29E4 == 0)
    {
        D_801B29E0 += 1;
    }
}
