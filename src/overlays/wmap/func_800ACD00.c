#include "wmap_sequence_runtime.h"
#include "common.h"

extern u32 D_801B2EA0;
extern s32 D_801B2EA4;
extern void (*D_800D6F2C[])(void);
extern void func_80066F9C(u8* obj, s32 a1, s32 a2, s32 a3, s32 a4);
extern u8 D_800D93F4[];
extern u8 D_801399D0[];
extern s32 D_80182D6C;

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_800ACC88(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2EA0 = 1;
        D_801B2EA4 = 1;
        return 1;
    }

    if (D_801B2EA0 < 0x4)
    {
        D_800D6F2C[D_801B2EA0]();
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
void func_800ACD00(void)
{
    D_801B2EA0 = 1;
    D_801B2EA4 = 1;
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void func_800ACD18(void)
{
    func_8006CC4C(D_800D93F4, D_801399D0);
    func_80066F9C(D_800D93F4, D_80182D6C, 0x24, 0x2, 0);
    if (--D_801B2EA4 == 0)
    {
        D_801B2EA0 += 1;
    }
}
