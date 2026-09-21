#include "common.h"

extern u32 D_801B28C8;
extern s32 D_801B28CC;
extern void (*D_800D59C0[])(void);
extern void func_800862C0(void);
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
s32 func_800861B0(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B28C8 = 1;
        D_801B28CC = 1;
        return 1;
    }

    if (D_801B28C8 < 0x4)
    {
        D_800D59C0[D_801B28C8]();
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
void func_80086228(void)
{
    D_801B28C8 = 1;
    D_801B28CC = 1;
}

/**
 * @brief Populate a world-map actor control block and schedule its spawn step.
 */
void func_80086240(void)
{
    D_801399B4 = D_8011F538;
    D_800D9344[0x6] = 0xF;
    *(s16*)&D_800D9344[0x10] = -1;
    *(s16*)&D_800D9344[0x26] = 2;
    *(s16*)&D_800D9344[0x22] = 0x81;
    *(s16*)&D_800D9344[0x2] = 0;
    *(s16*)&D_800D9344[0xE] = 0;
    *(s16*)&D_800D9344[0x24] = 1;
    D_801B28CC = 0xC0;
    D_801B28C8 += 1;
    func_800862C0();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void func_800862C0(void)
{
    func_8006CC4C(D_800D9344, D_801399B0);
    func_80066F9C(D_800D9344, D_8011CF4C, 0x18, 0xA, 0);
    if (--D_801B28CC == 0)
    {
        D_801B28C8 += 1;
    }
}
