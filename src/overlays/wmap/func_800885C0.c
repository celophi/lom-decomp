#include "wmap_effect_primitives.h"
#include "common.h"

extern u32 D_801B2948;
extern s32 D_801B294C;
extern void (*D_800D5B30[])(void);
extern u8 D_800DA7B8[];
extern u8 D_80139D68[];
extern s32 D_80139280;

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_80088548(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2948 = 1;
        D_801B294C = 1;
        return 1;
    }

    if (D_801B2948 < 0x6)
    {
        D_800D5B30[D_801B2948]();
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
void func_800885C0(void)
{
    D_801B2948 = 1;
    D_801B294C = 1;
}

/**
 * @brief Draw the world-map sprite via func_8006A2FC, then advance after the wait expires.
 */
void func_800885D8(void)
{
    func_8006A2FC(D_800DA7B8, D_80139D68, 0x32, 0, 0x7F, 0x2, 0, (s32)((u8*)D_80139280 + 0x50));
    if (--D_801B294C == 0)
    {
        D_801B2948 += 1;
    }
}
