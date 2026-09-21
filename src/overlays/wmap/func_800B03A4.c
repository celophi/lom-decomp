#include "common.h"

extern u32 D_801B2F48;
extern s32 D_801B2F4C;
extern void (*D_800D712C[])(void);
extern void func_800B043C(void);
extern u8 D_8011D538[];
extern u8* D_801399C4;
extern u8 D_800D939C[];

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_800B032C(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2F48 = 1;
        D_801B2F4C = 1;
        return 1;
    }

    if (D_801B2F48 < 0x4)
    {
        D_800D712C[D_801B2F48]();
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
void func_800B03A4(void)
{
    D_801B2F48 = 1;
    D_801B2F4C = 1;
}

/**
 * @brief Populate a world-map actor control block and schedule its spawn step.
 */
void func_800B03BC(void)
{
    D_801399C4 = D_8011D538;
    D_800D939C[0x6] = 0xF;
    *(s16*)&D_800D939C[0xE] = 2;
    *(s16*)&D_800D939C[0x10] = -1;
    *(s16*)&D_800D939C[0x26] = 8;
    *(s16*)&D_800D939C[0x2] = 0;
    *(s16*)&D_800D939C[0x22] = 0x81;
    *(s16*)&D_800D939C[0x24] = 0x81;
    D_801B2F4C = 0x24;
    D_801B2F48 += 1;
    func_800B043C();
}
