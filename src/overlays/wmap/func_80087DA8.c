#include "common.h"

extern u32 D_801B2920;
extern s32 D_801B2924;
extern void (*D_800D5AC8[])(void);
extern void func_80087E3C(void);
extern u8 D_8011F538[];
extern u8* D_801399B4;
extern u8 D_800D9344[];
extern void func_8006CC4C(u8* obj, u8* a1);
extern void func_80066F9C(u8* obj, s32 a1, s32 a2, s32 a3, s32 a4);
extern u8 D_801399B0[];
extern s32 D_8011CF4C;

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_80087D30(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2920 = 1;
        D_801B2924 = 1;
        return 1;
    }

    if (D_801B2920 < 0x4)
    {
        D_800D5AC8[D_801B2920]();
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
void func_80087DA8(void)
{
    D_801B2920 = 1;
    D_801B2924 = 1;
}

/**
 * @brief Populate a world-map actor control block and schedule its spawn step.
 */
void func_80087DC0(void)
{
    D_801399B4 = D_8011F538;
    D_800D9344[0x6] = 0xF;
    *(s16*)&D_800D9344[0x10] = -1;
    *(s16*)&D_800D9344[0x26] = 8;
    *(s16*)&D_800D9344[0x2] = 0;
    *(s16*)&D_800D9344[0xE] = 0;
    *(s16*)&D_800D9344[0x22] = 0x80;
    *(s16*)&D_800D9344[0x24] = 0;
    D_801B2924 = 0x8C;
    D_801B2920 += 1;
    func_80087E3C();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void func_80087E3C(void)
{
    func_8006CC4C(D_800D9344, D_801399B0);
    func_80066F9C(D_800D9344, D_8011CF4C, 0x1A, 0xA, 0);
    if (--D_801B2924 == 0)
    {
        D_801B2920 += 1;
    }
}
