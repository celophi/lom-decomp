#include "common.h"

extern u32 D_801B2640;
extern s32 D_801B2644;
extern void (*D_800D5250[])(void);
extern void func_80078B98(void);
extern u8 D_8011F538[];
extern u8* D_801399B4;
extern u8 D_800D9344[];

/**
 * @brief Dispatch the current world-map sequence step, seeding it first if requested.
 * @param arg0 Non-zero seeds the step counters before dispatching.
 * @return 1 if a step ran, 0 if the step index was out of range.
 */
s32 func_80078A90(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2640 = 1;
        D_801B2644 = 1;
    }

    if (D_801B2640 < 0x4)
    {
        D_800D5250[D_801B2640]();
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
void func_80078B00(void)
{
    D_801B2640 = 1;
    D_801B2644 = 1;
}

/**
 * @brief Populate a world-map actor control block and schedule its spawn step.
 */
void func_80078B18(void)
{
    D_801399B4 = D_8011F538;
    D_800D9344[0x6] = 0xF;
    *(s16*)&D_800D9344[0x10] = -1;
    *(s16*)&D_800D9344[0x26] = 4;
    *(s16*)&D_800D9344[0x22] = 0x80;
    *(s16*)&D_800D9344[0x2] = 0;
    *(s16*)&D_800D9344[0xE] = 0;
    *(s16*)&D_800D9344[0x24] = 0xFC;
    D_801B2644 = 0xAF;
    D_801B2640 += 1;
    func_80078B98();
}
