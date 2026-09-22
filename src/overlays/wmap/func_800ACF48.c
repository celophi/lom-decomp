#include "wmap_sequence_runtime.h"
#include "common.h"

extern u32 D_801B2EB0;
extern s32 D_801B2EB4;
extern void (*D_800D6F4C[])(void);
extern void func_80066F9C(u8* obj, s32 a1, s32 a2, s32 a3, s32 a4);
extern u8 D_800D939C[];
extern u8 D_801399C0[];
extern s32 D_80182D84;

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_800ACED0(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2EB0 = 1;
        D_801B2EB4 = 1;
        return 1;
    }

    if (D_801B2EB0 < 0x4)
    {
        D_800D6F4C[D_801B2EB0]();
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
void func_800ACF48(void)
{
    D_801B2EB0 = 1;
    D_801B2EB4 = 1;
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void func_800ACF60(void)
{
    func_8006CC4C(D_800D939C, D_801399C0);
    func_80066F9C(D_800D939C, D_80182D84, 0x23, 0x2, 0);
    if (--D_801B2EB4 == 0)
    {
        D_801B2EB0 += 1;
    }
}
