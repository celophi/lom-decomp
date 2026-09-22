#include "wmap_effect_primitives.h"
#include "common.h"

extern u32 D_801B2DC0;
extern s32 D_801B2DC4;
extern void (*D_800D6A54[])(void);
extern u8 D_800DA448[];
extern u8 D_80139CC8[];
extern s32 D_80139280;

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_800A2F38(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2DC0 = 1;
        D_801B2DC4 = 1;
        return 1;
    }

    if (D_801B2DC0 < 0x6)
    {
        D_800D6A54[D_801B2DC0]();
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
void func_800A2FB0(void)
{
    D_801B2DC0 = 1;
    D_801B2DC4 = 1;
}

/**
 * @brief Prime the frame, draw the world-map sprite, then advance after the wait expires.
 */
void func_800A2FC8(void)
{
    func_8006AEE0();
    func_8006A2FC(D_800DA448, D_80139CC8, 0x28, 0xFF, 0x1, 0x8, 0, (s32)((u8*)D_80139280 + 0x50));
    if (--D_801B2DC4 == 0)
    {
        D_801B2DC0 += 1;
    }
}
