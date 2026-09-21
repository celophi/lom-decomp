#include "common.h"

extern u32 D_801B2880;
extern s32 D_801B2884;
extern void (*D_800D58E0[])(void);
extern void func_80084B48(void);
extern u8 D_8011D538[];
extern u8* D_801399B4;
extern u8 D_800D9344[];

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_80084A3C(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2880 = 1;
        D_801B2884 = 1;
        return 1;
    }

    if (D_801B2880 < 0x4)
    {
        D_800D58E0[D_801B2880]();
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
void func_80084AB4(void)
{
    D_801B2880 = 1;
    D_801B2884 = 1;
}

/**
 * @brief Populate a world-map actor control block and schedule its spawn step.
 */
void func_80084ACC(void)
{
    D_801399B4 = D_8011D538;
    D_800D9344[0x6] = 0xF;
    *(s16*)&D_800D9344[0x10] = -1;
    *(s16*)&D_800D9344[0x26] = 8;
    *(s16*)&D_800D9344[0x2] = 0;
    *(s16*)&D_800D9344[0xE] = 0;
    *(s16*)&D_800D9344[0x22] = 0x80;
    *(s16*)&D_800D9344[0x24] = 0x80;
    D_801B2884 = 0x79;
    D_801B2880 += 1;
    func_80084B48();
}
