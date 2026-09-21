#include "common.h"

extern u32 D_801B2A88;
extern s32 D_801B2A8C;
extern void (*D_800D5F78[])(void);
extern void func_80090998(void);
extern u8 D_8011F538[];
extern u8* D_801399B4;
extern u8 D_800D9344[];

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_80090888(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2A88 = 1;
        D_801B2A8C = 1;
        return 1;
    }

    if (D_801B2A88 < 0x6)
    {
        D_800D5F78[D_801B2A88]();
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
void func_80090900(void)
{
    D_801B2A88 = 1;
    D_801B2A8C = 1;
}

/**
 * @brief Populate a world-map actor control block and schedule its spawn step.
 */
void func_80090918(void)
{
    D_801399B4 = D_8011F538;
    D_800D9344[0x6] = 0xF;
    *(s16*)&D_800D9344[0x10] = -1;
    *(s16*)&D_800D9344[0x26] = 0x10;
    *(s16*)&D_800D9344[0x22] = 0x81;
    *(s16*)&D_800D9344[0x2] = 0;
    *(s16*)&D_800D9344[0xE] = 0;
    *(s16*)&D_800D9344[0x24] = 1;
    D_801B2A8C = 0xA0;
    D_801B2A88 += 1;
    func_80090998();
}
