#include "wmap_effect_primitives.h"
#include "common.h"

extern u32 D_801B2D28;
extern s32 D_801B2D2C;
extern void (*D_800D6884[])(void);
extern u8 D_800DA8C0[];
extern u8 D_80139D98[];
extern s32 D_80139280;

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_8009FBD0(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2D28 = 1;
        D_801B2D2C = 1;
        return 1;
    }

    if (D_801B2D28 < 0x6)
    {
        D_800D6884[D_801B2D28]();
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
void func_8009FC48(void)
{
    D_801B2D28 = 1;
    D_801B2D2C = 1;
}

/**
 * @brief Prime the frame, draw the world-map sprite, then advance after the wait expires.
 */
void func_8009FC60(void)
{
    func_8006AEE0();
    func_8006A2FC(D_800DA8C0, D_80139D98, 0x30, 0xFF, 0x1, 0x8, 0, (s32)((u8*)D_80139280 + 0x50));
    if (--D_801B2D2C == 0)
    {
        D_801B2D28 += 1;
    }
}
