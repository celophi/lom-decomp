#include "common.h"

extern u32 D_801B2BA0;
extern s32 D_801B2BA4;
extern void (*D_800D6308[])(void);
extern void func_800967FC(void);
extern u8 D_8011D538[];
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
s32 func_800966F0(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B2BA0 = 1;
        D_801B2BA4 = 1;
        return 1;
    }

    if (D_801B2BA0 < 0x4)
    {
        D_800D6308[D_801B2BA0]();
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
void func_80096768(void)
{
    D_801B2BA0 = 1;
    D_801B2BA4 = 1;
}

/**
 * @brief Populate a world-map actor control block and schedule its spawn step.
 */
void func_80096780(void)
{
    D_801399B4 = D_8011D538;
    D_800D9344[0x6] = 0xF;
    *(s16*)&D_800D9344[0x10] = -1;
    *(s16*)&D_800D9344[0x26] = 8;
    *(s16*)&D_800D9344[0x2] = 0;
    *(s16*)&D_800D9344[0xE] = 0;
    *(s16*)&D_800D9344[0x22] = 0x80;
    *(s16*)&D_800D9344[0x24] = 0;
    D_801B2BA4 = 0xAC;
    D_801B2BA0 += 1;
    func_800967FC();
}

/**
 * @brief Draw the world-map sprite this frame, then advance after the wait expires.
 */
void func_800967FC(void)
{
    func_8006CC4C(D_800D9344, D_801399B0);
    func_80066F9C(D_800D9344, D_8011CF4C, 0x17, 0x8, 0);
    if (--D_801B2BA4 == 0)
    {
        D_801B2BA0 += 1;
    }
}
