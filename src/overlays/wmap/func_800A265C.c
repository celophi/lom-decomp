#include "common.h"

extern u32 D_801B2D90;
extern s32 D_801B2D94;
extern void (*D_800D69D4[])(void);
extern void func_800A26F0(void);
extern u8 D_80123538[];
extern u8* D_801399C4;
extern u8 D_800D939C[];

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_800A25E4(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2D90 = 1;
        D_801B2D94 = 1;
        return 1;
    }

    if (D_801B2D90 < 0x4)
    {
        D_800D69D4[D_801B2D90]();
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
void func_800A265C(void)
{
    D_801B2D90 = 1;
    D_801B2D94 = 1;
}

/**
 * @brief Populate a world-map actor control block and schedule its spawn step.
 */
void func_800A2674(void)
{
    D_801399C4 = D_80123538;
    D_800D939C[0x6] = 0xF;
    *(s16*)&D_800D939C[0xE] = 1;
    *(s16*)&D_800D939C[0x10] = -1;
    *(s16*)&D_800D939C[0x2] = 0;
    *(s16*)&D_800D939C[0x26] = 0;
    *(s16*)&D_800D939C[0x22] = 0x7F;
    *(s16*)&D_800D939C[0x24] = 0x7F;
    D_801B2D94 = 0xBE;
    D_801B2D90 += 1;
    func_800A26F0();
}
