#include "common.h"

extern u32 D_801B2C40;
extern s32 D_801B2C44;
extern void (*D_800D64F0[])(void);
extern void func_8006AEE0(void);
extern void func_8006A2FC(u8* a0, u8* a1, s32 a2, s32 a3, s32 a4, s32 a5, s32 a6, s32 a7);
extern u8 D_800DB578[];
extern u8 D_80139FE8[];
extern s32 D_80139280;

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_8009954C(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2C40 = 1;
        D_801B2C44 = 1;
        return 1;
    }

    if (D_801B2C40 < 0x6)
    {
        D_800D64F0[D_801B2C40]();
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
void func_800995C4(void)
{
    D_801B2C40 = 1;
    D_801B2C44 = 1;
}

/**
 * @brief Prime the frame, draw the world-map sprite, then advance after the wait expires.
 */
void func_800995DC(void)
{
    func_8006AEE0();
    func_8006A2FC(D_800DB578, D_80139FE8, 0x28, 0xFF, 0x1, 0x8, 0, (s32)((u8*)D_80139280 + 0x78));
    if (--D_801B2C44 == 0)
    {
        D_801B2C40 += 1;
    }
}
