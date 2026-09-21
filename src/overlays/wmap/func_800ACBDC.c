#include "common.h"

extern u32 D_801B2E98;
extern s32 D_801B2E9C;
extern void (*D_800D6F1C[])(void);
extern void func_8006CC4C(u8* obj, u8* a1);
extern void func_80066F9C(u8* obj, s32 a1, s32 a2, s32 a3, s32 a4);
extern u8 D_800D9370[];
extern u8 D_801399B8[];
extern s32 D_80182D64;

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_800ACB64(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2E98 = 1;
        D_801B2E9C = 1;
        return 1;
    }

    if (D_801B2E98 < 0x4)
    {
        D_800D6F1C[D_801B2E98]();
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
void func_800ACBDC(void)
{
    D_801B2E98 = 1;
    D_801B2E9C = 1;
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void func_800ACBF4(void)
{
    func_8006CC4C(D_800D9370, D_801399B8);
    func_80066F9C(D_800D9370, D_80182D64, 0x21, 0x2, 0);
    if (--D_801B2E9C == 0)
    {
        D_801B2E98 += 1;
    }
}
