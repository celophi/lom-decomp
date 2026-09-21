#include "common.h"

extern u32 D_801B2EA8;
extern s32 D_801B2EAC;
extern void (*D_800D6F3C[])(void);
extern void func_8006CC4C(u8* obj, u8* a1);
extern void func_80066F9C(u8* obj, s32 a1, s32 a2, s32 a3, s32 a4);
extern u8 D_800D93C8[];
extern u8 D_801399C8[];
extern s32 D_80182D7C;

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_800ACDAC(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2EA8 = 1;
        D_801B2EAC = 1;
        return 1;
    }

    if (D_801B2EA8 < 0x4)
    {
        D_800D6F3C[D_801B2EA8]();
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
void func_800ACE24(void)
{
    D_801B2EA8 = 1;
    D_801B2EAC = 1;
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void func_800ACE3C(void)
{
    func_8006CC4C(D_800D93C8, D_801399C8);
    func_80066F9C(D_800D93C8, D_80182D7C, 0x11, 0x2, 0);
    if (--D_801B2EAC == 0)
    {
        D_801B2EA8 += 1;
    }
}
