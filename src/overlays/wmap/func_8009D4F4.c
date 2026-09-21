#include "common.h"

extern u32 D_801B2CC8;
extern s32 D_801B2CCC;
extern void (*D_800D671C[])(void);
extern void func_8009D58C(void);
extern u8 D_8011D538[];
extern u8* D_801399CC;
extern u8 D_800D93C8[];

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_8009D47C(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2CC8 = 1;
        D_801B2CCC = 1;
        return 1;
    }

    if (D_801B2CC8 < 0x4)
    {
        D_800D671C[D_801B2CC8]();
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
void func_8009D4F4(void)
{
    D_801B2CC8 = 1;
    D_801B2CCC = 1;
}

/**
 * @brief Populate a world-map actor control block and schedule its spawn step.
 */
void func_8009D50C(void)
{
    D_801399CC = D_8011D538;
    D_800D93C8[0x6] = 0xF;
    *(s16*)&D_800D93C8[0x10] = -1;
    *(s16*)&D_800D93C8[0x26] = 2;
    *(s16*)&D_800D93C8[0x22] = 0x81;
    *(s16*)&D_800D93C8[0x2] = 0;
    *(s16*)&D_800D93C8[0xE] = 0;
    *(s16*)&D_800D93C8[0x24] = 1;
    D_801B2CCC = 0x5A;
    D_801B2CC8 += 1;
    func_8009D58C();
}
