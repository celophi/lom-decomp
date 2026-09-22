#include "common.h"

extern u32 D_801B2448;
extern s32 D_801B244C;
extern void (*D_800D4D7C[])(void);
extern void func_800701DC(void);
extern s32 D_801B248C;

/**
 * @brief Dispatch the current world-map sequence step, seeding it first if requested.
 * @param arg0 Non-zero seeds the step counters before dispatching.
 * @return 1 if a step ran, 0 if the step index was out of range.
 */
s32 func_80070114(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2448 = 1;
        D_801B244C = 1;
    }

    if (D_801B2448 < 0x6)
    {
        D_800D4D7C[D_801B2448]();
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
void func_80070184(void)
{
    D_801B2448 = 1;
    D_801B244C = 1;
}

/**
 * @brief Clear the sub-flag, set the sequence parameter, advance the counter, and run the handler.
 */
void func_8007019C(void)
{
    D_801B248C = 0;
    D_801B244C = 0x3D;
    D_801B2448 += 1;
    func_800701DC();
}
